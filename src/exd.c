/** 
* @file
* @brief	Execution driven simulation & Interface with Simics tools and functions.
*/

#if (EXECUTION_DRIVEN != 0)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_ether.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <string.h>
#include <limits.h>

#include "exd.h"

void packet_dispatcher(list * list_sockets, list * list_packets);
void frame_packetizer(struct packet * pack, unsigned long FSIN_destination);
port_type generate_FSIN_packet(unsigned long src, unsigned long dst, long id_ethernet_frame);
//void generate_FSIN_phits(packet_t packet, port_type iport, unsigned long id_ethernet_frame, unsigned short id_packet_sequence);
void closing();
void die(char *);
void get_ip_addr(struct in_addr*,char*);
void get_hw_addr(u_char*,char*);
void get_simics_fsin_stats();
void clear_simics_fsin_stats();
void SwapBytes (void * b1, void * b2, unsigned short len);
unsigned short csum (unsigned short *buf, int nwords);

/* Estructura de un mensaje UDP/IP que llega por un socket de un proceso simics y que contiene GET_STATS o CLEAR STATS*/
/*when,count,h_dest,h_src,h_proto,ihl,version,tos,tot_len,id,frag_off,ttl,protocol,check,saddr,daddr,
  8,     2,    6,    6,     2,   4bit, 4bit,  1,   2,     2,  2,       1,   1,      2,    4,    4,
      Simics,            Ethernet,                                                                IP,

source,dest,len,check,action
 2,     2,   2,   2,   1
                  UDP, Payload
*/

/**
 * Packet dispatcher function.
 *
 * This function takes the traffic received from the SIMICS networtk interfaces & 
 * Esta funcion manda los paquetes recibidos y que han sido guardados en la lista de paquetes
 * por su correspondiente socket de destino, o sea, que despacha los paquetes recibidos a su destino.
 * @param list_sockets a list containing the connected sockets from SIMICS.
 * @param list_packets a list containing the data frames within FSIN.
 */
void packet_dispatcher(list * list_sockets, list * list_packets){

	struct packet * pack;
	char * buf;
	int escritor;
	unsigned short count, aux_len;
	struct arp_record * aux;
	int i=0,j=0;
	struct in_addr targ_in_addr;
	u_char targ_hw_addr[ETH_ALEN];
	char clear_stats = CLEAR_STATS, get_stats = GET_STATS;
		
	get_hw_addr(targ_hw_addr,TARG_HW_ADDR);
	get_ip_addr(&targ_in_addr,TARG_IP_ADDR);
		
    veces_que_entra_FSIN ++;
    while (TRUE){
	pack = (struct packet *) StartLoop(list_packets);
	//if(pack == NULL) printf ("no hay paquetes en la lista de paquetes\n");
    	
	for (; pack; pack = (struct packet *) GetNext(list_packets)){
		// Ver si la trama ethernet ha sido tratada anteriormente
		// para no volver a tratarla
		//printf("Dispatcher: trama %d, encoladas: %d\n", pack->id_ethernet_frame, ElementsInList(list_packets));
		//if (pack->num_FSIN_packets != 0) continue;
		buf = pack->buffer;
		count = pack->length;
		escritor = pack->socket;
		if (!memcmp(buf + sizeof(when) + sizeof(count), broadcast, ETH_ALEN)){
			// el paquete es un broadcast arp hay que mandarlo a todos
			// habria que insertar el mismo paquete en la lista de paquetes
			// pero destinado a cada uno de los nodos repetidas veces
			// todavia no lo implementamos y simplemente se manda directamente
			// como antes en la version standalone de SIMICS-TrGen
			aux = (struct arp_record *) StartLoop(list_sockets);
			for (; aux; aux = (struct arp_record *) GetNext(list_sockets)){
				//printf("Sockets en lista: %d\n", ElementsInList(list_sockets));
				if (aux->socket != sock && aux->socket != escritor){
					write(aux->socket, buf, count + sizeof(when) + sizeof(count));
					if (DEBUG >=5) printf("escribe un ARP broadcast en socket:%d, paquete:%d\n", aux->socket, ++aux->paquetes);
				}
			}
			// Destruir la trama ethernet
			RemoveFromList(list_packets, pack);
			free(pack->buffer);
			free(pack);
			
		} else if(!memcmp(buf + sizeof(when) + sizeof(count), targ_hw_addr, ETH_ALEN) &&
				!memcmp(buf + sizeof(when) + sizeof(count) + ETH_ALEN*2 + 18, &targ_in_addr, IP_ADDR_LEN) &&
				!memcmp(buf + sizeof(when) + sizeof(count) + ETH_ALEN*2 + 30, &clear_stats, sizeof(char))){
			printf("Special ICMP packet received, cleaning FSIN statistics\n");
			/* It is a special packet for us to clear FSIN statistics (clear_fsin_stats)*/
			/* Devolver el mensaje que ha llegado al proceso que lo ha mandado ya que lo esta esperando */
			aux = (struct arp_record *) StartLoop(list_sockets);
			for (; aux; aux = (struct arp_record *) GetNext(list_sockets)){
				if(!memcmp(aux->StationAddress, buf + sizeof(when) + sizeof(count) + ETH_ALEN, ETH_ALEN)){
				/* Encontrado el socket destino del mensaje */
					/* Ahora hay que reformatear el mensaje para que le llegue al destino */	
					/* Primero intercambiar las MAC origen y destino */
					SwapBytes(buf + sizeof(when) + sizeof(count), buf + sizeof(when) + sizeof(count) + ETH_ALEN, ETH_ALEN);
					/* Luego intercambiar las IP origen y destino */
					SwapBytes(buf + sizeof(when) + sizeof(count) + 26, 
						buf + sizeof(when) + sizeof(count) + 26 +IP_ADDR_LEN, IP_ADDR_LEN);
					/* Intercambiar los puertos origen y destino aunque sean el mismo en nuestro caso */
					SwapBytes(buf + sizeof(when) + sizeof(count) + 34, 
						buf + sizeof(when) + sizeof(count) + 36, 2);
					/* Calcular checksum de la cabecera udp y el payload */
					memcpy(&aux_len, buf + sizeof(when) + sizeof(count) + 38, 2);
					*(unsigned short *)(buf + sizeof(when) + sizeof(count) + 40) = htons(
						csum ((unsigned short *) (buf + sizeof(when) + sizeof(count) + 34), ntohs(aux_len) >> 1));
					/* calcular el checksum de la cabecera ip y de todo el contenido del paquete ip 
					o sea la cabecera udp y el payload */
					memcpy(&aux_len, buf + sizeof(when) + sizeof(count) + 16, 2);
					*(unsigned short *)(buf + sizeof(when) + sizeof(count) + 24) = htons(
						csum ((unsigned short *) buf + sizeof(when) + sizeof(count) + 14, ntohs(aux_len) >> 1));
					write(aux->socket, buf, count + sizeof(when) + sizeof(count));
					if (DEBUG >=5) printf("escribe respuesta a CLEAR_STATS en socket:%d\n", aux->socket);
					break;
				}
			}
			/* Inicializar las estadisticas */
			clear_simics_fsin_stats();
			
			// Destruir la trama ethernet
			RemoveFromList(list_packets, pack);
			free(pack->buffer);
			free(pack);
		
		} else if(!memcmp(buf + sizeof(when) + sizeof(count), targ_hw_addr, ETH_ALEN) &&
				!memcmp(buf + sizeof(when) + sizeof(count) + ETH_ALEN*2 + 18, &targ_in_addr, IP_ADDR_LEN) &&
				!memcmp(buf + sizeof(when) + sizeof(count) + ETH_ALEN*2 + 30, &get_stats, sizeof(char))){ 
			/* It is a special packet for us to print FSIN statistics (get_fsin_stats)*/
			/* Devolver el mensaje que ha llegado al proceso que lo ha mandado ya que lo esta esperando */
			aux = (struct arp_record *) StartLoop(list_sockets);
			for (; aux; aux = (struct arp_record *) GetNext(list_sockets)){
				if(!memcmp(aux->StationAddress, buf + sizeof(when) + sizeof(count) + ETH_ALEN, ETH_ALEN)){
				/* Encontrado el socket origen del mensaje */
					/* Ahora hay que reformatear el mensaje para que le llegue de vuelta al origen */	
					/* Primero intercambiar las MAC origen y destino */
					SwapBytes(buf + sizeof(when) + sizeof(count), buf + sizeof(when) + sizeof(count) + ETH_ALEN, ETH_ALEN);
					/* Luego intercambiar las IP origen y destino */
					SwapBytes(buf + sizeof(when) + sizeof(count) + 26, 
						buf + sizeof(when) + sizeof(count) + 26 +IP_ADDR_LEN, IP_ADDR_LEN);
					/* Intercambiar los puertos origen y destino aunque sean el mismo en nuestro caso */
					SwapBytes(buf + sizeof(when) + sizeof(count) + 34, 
						buf + sizeof(when) + sizeof(count) + 36, 2);
					/* Calcular checksum de la cabecera udp y el payload */
					memcpy(&aux_len, buf + sizeof(when) + sizeof(count) + 38, 2);
					*(unsigned short *)(buf + sizeof(when) + sizeof(count) + 40) = htons(
						csum ((unsigned short *) buf + sizeof(when) + sizeof(count) + 34, ntohs(aux_len) >> 1));
					/* calcular el checksum de la cabecera ip y de todo el contenido del paquete ip 
					o sea la cabecera udp y el payload */
					memcpy(&aux_len, buf + sizeof(when) + sizeof(count) + 16, 2);
					*(unsigned short *)(buf + sizeof(when) + sizeof(count) + 24) = htons(
						csum ((unsigned short *) buf + sizeof(when) + sizeof(count) + 14, ntohs(aux_len) >> 1));
					write(aux->socket, buf, count + sizeof(when) + sizeof(count));
					if (DEBUG >=5) printf("escribe respuesta a GET_STATS en socket:%d\n", aux->socket);
					break;
				}
			}
			/* Sacar por pantalla las estadisticas */
			get_simics_fsin_stats();

			// Destruir la trama ethernet
			RemoveFromList(list_packets, pack);
			free(pack->buffer);
			free(pack);
		
		} else if (!run_FSIN){
			/* No se ha recibido el mensaje de CLEAR_STATS aun*/
			/* Hay que enviar el mensaje directamente al destino */
			aux = (struct arp_record *) StartLoop(list_sockets);
			for (; aux; aux = (struct arp_record *) GetNext(list_sockets)){
				if(!memcmp(aux->StationAddress, buf + sizeof(when) + sizeof(count), ETH_ALEN)){
				/* Encontrado el socket destino del mensaje */
					write(aux->socket, buf, count + sizeof(when) + sizeof(count));
					if (DEBUG >=5) printf("escribe mensaje en el destino directamente en socket:%d\n", aux->socket);
					break;
				}
			}

			// Destruir la trama ethernet
			RemoveFromList(list_packets, pack);
			free(pack->buffer);
			free(pack);

		} else{ /* el mensaje no es un broadcast ni un mensaje de peticion de estadisticas, 
			se ha recibido el mensaje de CLEAR_STATS, hay que inyectarlo en FSIN */
			aux = (struct arp_record *) StartLoop(list_sockets);
			for (; aux; aux = (struct arp_record *) GetNext(list_sockets)){
				if (DEBUG>=10){
					for (i=0; i<6; i++){
						printf("%02x ", aux->StationAddress[i]&0xff);
					}
					printf(" ");
					for (i=0; i<6; i++){
						printf("%02x ", buf[i + sizeof(count)]&0xff);
					}
					printf("\n");
				}
				if(!memcmp(aux->StationAddress, buf + sizeof(when) + sizeof(count), ETH_ALEN)){
					frame_packetizer(pack, aux->FSIN_node_number);
					break;
				}
			}
		}
	}
	if (run_FSIN){
	// realizar tantos ciclos de simulacion como indique la relacion entre Simics y FSIN
		if (list_packets->emptyList){
			sim_clock += fsin_cycle_run;
			if (sim_clock%pinterval == 0) results_partial();
		}
		else {
			for (i=0; i < fsin_cycle_run; i++) {
			
				// inyectar los phits en las colas de inyeccion
				for (j=0; j < nprocs; j++) data_injection(j);
		
				data_movement(FALSE);
				sim_clock++;
				if (sim_clock%pinterval == 0) results_partial();
			}
		}
	}
	return;
    }
}


/**
 * funcion get_simics_fsin_stats
 * Esta funcion acaba con el conteo de las estadisticas y saca por pantalla las estadisticas de la ejecucion
 * This function prints the execution driven simulation stats from FSIN, & returns to direct dispatching mode.
 */
void get_simics_fsin_stats(){
    
    time(&end_time);
	save_batch_results();
	print_results(start_time, end_time);
	time(&start_time);
	//reset_stats();
	simics_cycle_run = SIMICS_CYCLE_RUN_DEFAULT;
	run_FSIN = 0;
	printf("veces que entra FSIN:%lld\n", veces_que_entra_FSIN);
	printf("timestamp_init:%lld\n", timestamp_init);
	printf("timestamp_end:%lld\n", timestamp_end);
	printf("timestamp_end - timestamp_init:%lld\n", timestamp_end - timestamp_init);
}

/**
 * funcion clear_simics_fsin_stats
 * Esta funcion limpia las estadisticas y comienza el conteo de las estadisticas
 * This function clears FSIN stats & and starts using the simulated network for the traffic provided by SIMICS.
 */
void clear_simics_fsin_stats(){
	run_FSIN = 1;
    veces_que_entra_FSIN = 0;
	timestamp_init = 0;
	timestamp_end = 0;
    time(&start_time);
	reset_stats();
	simics_cycle_run = simics_cycle_relation;
}

/**
 * funcion frame_packetizer
 * Esta funcion crea los paquetes en que se divide la trama ethernet
 * tanto los paquetes que se guardan en struct packet como los de FSIN.
 * Los paquetes de FSIN se convierten en phits y se inyectan en los buferes
 * de inyeccion de los routers
 * This function divides ethernet data frames into FSIN packets & inject them into the simulated network.
 * @param pack A list with all the data frames within FSIN.
 * @param FSIN_destination Node 
 */
void frame_packetizer(struct packet * pack, unsigned long FSIN_destination){

	int i=0;
	struct arp_record * aux;
	port_type iport;
	long id_trama;

	if (pack->num_FSIN_packets != 0) {
		// En esta trama hay algun/os paquete/s que no han conseguido ser inyectados
		// porque el buffer de inyeccion estaba lleno, intentar inyectarlos otra vez
		for (i = 0; i < pack->num_FSIN_packets; i++){
			if (!pack->FSIN_packets[i].packet_generated){
				// generar paquete FSIN
				id_trama = (pack->id_ethernet_frame << num_bits_packet_sequence) | i;
				iport = generate_FSIN_packet(pack->FSIN_node_source, pack->FSIN_node_destination, id_trama);
				if (iport != NULL_PORT) {
					if (DEBUG >=5) printf("frame_packetizer paquete %i de %d de la trama %d regenerado\n", i, pack->num_FSIN_packets, pack->id_ethernet_frame);
					pack->FSIN_packets[i].packet_generated = 1;
				}
				else{
					// El paquete no cabia en el buffer de inyeccion y hay que deshacer la creacion de paquetes
					// y dejar que se inyecte mas tarde cuando haya sitio
					continue;
				}				
			}
		}
		return;
	}
		
	// completar la inicializacion de la trama previa a la paquetizacion de la misma
	//printf("PACKET_SIZE_IN_BYTES %d\n", PACKET_SIZE_IN_BYTES);
	pack->num_FSIN_packets = (unsigned short)ceil((double)pack->length / PACKET_SIZE_IN_BYTES);
	if (pack->num_FSIN_packets == 0) pack->num_FSIN_packets = 1;
	//printf("pack->num_FSIN_packets %u \n",pack->num_FSIN_packets);
	pack->num_packets_to_come = pack->num_FSIN_packets;
	pack->FSIN_node_destination = FSIN_destination;
	//printf("pack->FSIN_node_destination  %u \n",pack->FSIN_node_destination);
	pack->FSIN_packets = (struct FSIN_packet *)malloc (sizeof(struct FSIN_packet) * pack->num_FSIN_packets);
	for (i = 0; i < pack->num_FSIN_packets; i++){
		// inicializar cada paquete de la trama
		pack->FSIN_packets[i].packet_generated = 0;
		pack->FSIN_packets[i].number_phits = packet_size_in_phits;
		pack->FSIN_packets[i].tail_arrived = 0;
		// calcular el nodo origen en FSIN de la trama
		aux = (struct arp_record *) StartLoop(lista);
		for (; aux; aux = (struct arp_record *) GetNext(lista)){
			if( aux->socket == pack->socket){
				pack->FSIN_node_source = aux->FSIN_node_number;
				//printf("pack->FSIN_node_source  %u \n",pack->FSIN_node_source);
				break;
			}
		}
		// generar paquete FSIN
		id_trama = (pack->id_ethernet_frame << num_bits_packet_sequence) | i;
		iport = generate_FSIN_packet(pack->FSIN_node_source, pack->FSIN_node_destination, id_trama);
		if (iport != NULL_PORT) {
			pack->FSIN_packets[i].packet_generated = 1;
		}
		else{
			// El paquete no cabia en el buffer de inyeccion y hay que deshacer la creacion de paquetes
			// y dejar que se inyecte mas tarde cuando haya sitio
			//break;
		}
	}
}

/**
 * funcion generate_FSIN_packet 
 * Esta funcin rellena la estructura que contiene un paquete de FSIN 
 * Es analoga a generate_pkt de FSIN, pero no tiene que usar ningun tipo de 
 * distribucion para calcular el destino del paquete, porque ya viene dado por
 * la trama ethernet. El paquete no se genera sinteticamente
 */
port_type generate_FSIN_packet(unsigned long src, unsigned long dst, long id_ethernet_frame) {
	double aux;
	inj_queue *qi;
	port_type iport;
	packet_t packet, *packet_aux;
	unsigned long pkt;

	packet.to = dst;
	packet.from = src;
	packet.size = packet_size_in_phits; // pkt_len;
	packet.tt = sim_clock;
	inj_phit_count += packet.size;
	packet.rr = calc_rr(packet.from, packet.to);
	packet.inj_time = sim_clock;  // Some additional info
	packet.n_hops = 0;
	packet.id_trama = id_ethernet_frame;
	
	iport = select_input_port(packet.from, packet.to);
	qi = &(network[packet.from].qi[iport]);
	if (inj_queue_space(qi) < packet.size) {
		// Se pierden tramas porque se quedan atrapadas en FSIN porque sus paquetes se desechan
		/*if (!drop_packets) {
			network[src].saved_packet = *packet;
			network[src].pending_packet = TRUE;
		}
		else {
			dropped_count++;
			dropped_phit_count += packet->size;
		}*/
		return NULL_PORT;
	}
	pkt=get_pkt();
	packet_aux = &pkt_space[pkt];
	*packet_aux = packet;
	// generar los phits por cada paquete y dejarlos en el bufer de inyeccion
	generate_phits(pkt, iport);

	return iport;
}

/**
 * Funcion que comprueba si ha llegado un paquete y una trama depues de que haya
 * llegado un phit a su destino en FSIN
 * Los parametros son el nodo al que ha llegado el phit y el phit en cuestion
 */
void SIMICS_phit_away(long i, phit ph){
	struct packet * pack;
	char * buf;
	unsigned short count;
	struct arp_record * aux;
	double percent;
	packet_t * FSIN_pkt;
	int j=0;
	percent=0.0f;
	
	FSIN_pkt = &pkt_space[ph.packet];
	pack = (struct packet *) StartLoop(list_packets);
	for (; pack; pack = (struct packet *) GetNext(list_packets)){
		if (pack->id_ethernet_frame == (FSIN_pkt->id_trama & mask_eth_id_ethernet_frame) >> num_bits_packet_sequence) {
			// Cuando se llama a esta funcion ya llega el TAIL del paquete y los phits siempre
			// llegan en orden
			// Si el phit es la cola poner el indicador de cola llegada a TRUE
			pack->FSIN_packets[FSIN_pkt->id_trama & mask_eth_id_packet_sequence].tail_arrived = TRUE;
			// Han llegado todos los phits del paquete. Disminuir la lista de paquetes por llegar
			pack->num_packets_to_come --;
			// Ver si han llegado todos los paquetes
			if (pack->num_packets_to_come == 0){
				// Han llegado todos los paquetes de la trama.
				// Buscar el nodo destino SIMICS dado el nodo destino FSIN
				aux = (struct arp_record *) StartLoop(lista);
				for (; aux; aux = (struct arp_record *) GetNext(lista)){
					if( aux->FSIN_node_number == i){
						break;
					}
				}
				if (aux == NULL) {
					printf("Error grave en SIMICS_phit_away: no se encuentra el nodo SIMICS a partir del nodo FSIN %d\n", i);
					closing();
					exit(-1);
				}
				// Enviar la trama por el socket de destino de la misma
				buf = pack->buffer;
				count = pack->length;
				aux->paquetes ++;
				if (DEBUG >=5) printf("escribe en socket:%d paquete:%d\n", aux->socket, aux->paquetes);
				// Se calcula si el mensaje que acaba de llegar  hay que retrasarlo 
				// num_periodos_espera en ciclos Simics
				if (num_periodos_espera != 0){
					percent = ((double)rand()) / RAND_MAX;
					if (percent <= WAIT_PERCENT){
						if (DEBUG >=2) printf("toca esperar:%d num_periodos_espera\n", num_periodos_espera);
						memcpy(buf, (void *)&when, sizeof(when));
					}
					else {
						memset(buf, 0, sizeof(when));
					}
				}
				else memset(buf, 0, sizeof(when));
				memcpy(buf + sizeof(when), (void *)&count, sizeof(count));
				write(aux->socket, buf, count + sizeof(when) + sizeof(count));

				// Borrar la lista de phits y paquetes de la trama que se acaba de enviar
				// La lista de paquetes es un array de tamao fijo. Los phits de cada paquete
				// no hace falta liberarlos, ya que se representan como un contador
				free(pack->FSIN_packets);
				// Borrar la trama ethernet de la lista de tramas pendientes de entrega
				// Destruir la trama ethernet
				RemoveFromList(list_packets, pack);
				if (DEBUG >=5) printf("SIMICS_phit_away: Quita %d, quedan: %d\n", pack->id_ethernet_frame, ElementsInList(list_packets));
				free(pack->buffer);
				free(pack);
				
				break;
			}
		}
	}
}


/**
 * function closing.
 * cierra sockets abiertos y borra las listas creadas, paquetes y conexiones
 * This function closes open sockets & destroy data structures used in execution guided simulation.
 */
void closing(){
	struct arp_record * aux_recorrido;
	struct packet * pack;
	close(sock);
	//close(sock_connect_central);
	//close(central);
	aux_recorrido = (struct arp_record *) StartLoop(lista);
	for (; aux_recorrido; aux_recorrido = (struct arp_record *) GetNext(lista)){
		close(aux_recorrido->socket);
		printf("se ha quitado de la lista un socket abierto:%d\n", aux_recorrido->socket);
		RemoveFromList(lista, (void *) aux_recorrido);
		free(aux_recorrido);
	}
	DestroyList(&lista);
	pack = (struct packet *) StartLoop(list_packets);
	//if(pack == NULL) printf ("no hay paquetes en la lista de paquetes\n");
	for (; pack; pack = (struct packet *) GetNext(list_packets)){
		RemoveFromList(list_packets, pack);
		free(pack->buffer);
		free(pack);
	}
	DestroyList(&list_packets);
	printf("sockets cerrados y listas vacias\n");
}

/**
 * Signal handler.
 * 
 * @param sig The SIGNAL id.
 */
void thread_signal_handler (int sig){
	if (sig == SIGUSR1){
		received_terminate_signal = 1;
		perror("ha llegado la senal de finalizacion de thread");
	}else if (sig == SIGINT){
		received_terminate_signal = 1;
		perror("ha llegado SIGINT al thread");
	}else if (sig == SIGTERM){
		received_terminate_signal = 1;
		perror("ha llegado SIGTERM al thread");
	}else if (sig == SIGHUP){
		received_terminate_signal = 1;
		perror("ha llegado SIGHUP al thread");
	}
	return;
}

/**
 * funcion close_input_reading_thread
 * Esta funcion se encarga de cerrar el thread que lee continuamente de la
 * entrada estandar el retardo de interconexion
 * Orden Parametros: puntero donde se encuentra el Identificador del thread a cerrar
 */
int close_input_reading_thread(pthread_t * idHilo){
	/* status devuelto por el thread que se cierra*/
	void * status;
	received_terminate_signal = 1;
	pthread_kill(*idHilo, SIGUSR1);
	pthread_join(*idHilo, &status);
	//pthread_mutex_destroy(&lock);
	if (status != 0){
		perror ("El thread ha devuelto un error al acabar");
		return (int) status;
	}
	return 0;
}

/**
 * funcion input_reading_thread 
 * esta funcion lee continuamente el parametro retardo de interconexion
 * de la entrada estandar
 * Orden Parametros:  ninguno
 */
void * input_reading_thread(void *arg) {
	char buf[50];
	char * s;
	int i=0;
	struct	sigaction	siginfo;
	char * name;
	char * value;
		
	//configuramos las seales a recibir
	sigemptyset(&(siginfo.sa_mask));
	sigaddset(&(siginfo.sa_mask),SIGUSR1);
	pthread_sigmask(SIG_UNBLOCK, &(siginfo.sa_mask), NULL);
	siginfo.sa_handler = thread_signal_handler;
	siginfo.sa_flags = 0;
	sigaction(SIGUSR1,&siginfo,NULL);
	for(i=0; i < 50; i++) buf[i]='\0';
	while(s=fgets(buf, 50, stdin)){
		if(s==NULL || received_terminate_signal || SIGPIPE_caught) {
			printf("recibido received_terminate_signal o SIGPIPE_caught o s == NULL\n");
			pthread_exit((void *)0);
		}
		else {
			if(buf[0] != '\n' && buf[0] != '#') {
				if(buf[strlen(buf) - 1] == '\n')
					buf[strlen(buf) - 1] = '\0';
			}
			name = (char *)strtok(buf, "=");
			if (!strncmp("pinterval", name, strlen(name))){
				value = (char *)strtok(NULL, "=");
				if (value == NULL) {
					printf("formato incorrecto: pinterval=number, ahora pinterval=%lu\n", pinterval);
					for(i=0; i < 50; i++) buf[i]='\0';	
					continue;
				}
				pinterval = atol(value);
				printf("pinterval=%"PRINT_CLOCK"\n", pinterval);
				for(i=0; i < 50; i++) buf[i]='\0';			
			}
			else if (!strncmp("plevel", name, strlen(name))){
				value = (char *)strtok(NULL, "=");
				if (value == NULL) {
					printf("formato incorrecto: plevel=number, ahora plevel=%lu\n", plevel);
					for(i=0; i < 50; i++) buf[i]='\0';	
					continue;
				}
				plevel = atol(value);
				printf("plevel=%lu\n", plevel);
				for(i=0; i < 50; i++) buf[i]='\0';			
			}
			else if (!strncmp("exit", name, strlen(name))){
				received_terminate_signal = 1;
				closing();
				signal(SIGINT, NULL);
				exit (0);
				//pthread_exit((void *)0);
			}
			else if (!strncmp("num_periodos_espera", name, strlen(name))){
				value = (char *)strtok(NULL, "=");
				if (value == NULL) {
					printf("formato incorrecto: num_periodos_espera=number, ahora num_periodos_espera=%lu\n", num_periodos_espera);
					for(i=0; i < 50; i++) buf[i]='\0';	
					continue;
				}
				num_periodos_espera = atol(value);
				//Ahora solo se esperan num_priodos_espera en ciclos Simics
				//when = periodo * num_periodos_espera;
				when = num_periodos_espera;
				printf("num_periodos_espera=%lu\n", num_periodos_espera);
				for(i=0; i < 50; i++) buf[i]='\0';
			}
        	  	else if (!strncmp("clear_stats", name, strlen(name))){
				clear_simics_fsin_stats();
                		printf("stats cleared\n");
			}
			else if (!strncmp("fsin_cycle_relation", name, strlen(name))){
				value = (char *)strtok(NULL, "=");
				if (value == NULL) {
					printf("formato incorrecto: fsin_cycle_relation=number, ahora fsin_cycle_relation=%lu\n", 
                                              fsin_cycle_relation);
 					for(i=0; i < 50; i++) buf[i]='\0';	
					continue;
				}
				fsin_cycle_relation = atol(value);
				fsin_cycle_run = fsin_cycle_relation;
				printf("fsin_cycle_relation=%lu\n", fsin_cycle_relation);
				for(i=0; i < 50; i++) buf[i]='\0';
			}
			else printf("input_reading_thread: Unknown option %s\n", name);
		}
	}
}


/**
 * funcion signal_handler
 * esta funcion se encarga de atender las seales recibidas y de comunicar
 * que se debe acabar el thread de lectura de la entrada estandar
 * Orden Parametros:  la seal recibida
 */
void signal_handler (int sig){
	if (sig == SIGPIPE){
		SIGPIPE_caught = 1;
		perror("ha llegado la senal de extremo de socket cerrado");
	}else if (sig == SIGINT){
		received_terminate_signal = 1;
		perror("ha llegado SIGINT");
	}else if (sig == SIGTERM){
		received_terminate_signal = 1;
		perror("ha llegado SIGTERM");
	}else if (sig == SIGHUP){
		received_terminate_signal = 1;
		perror("ha llegado SIGHUP");
	}
	perror("llamando a closing");
	closing();
	perror("cerrando el thread");
	close_input_reading_thread(&idHilo);
	exit(0);
}


/**
 * Funcion de inicializacion para la Simulacion conducida por la ejecucion
 * Parametros:
 * long fsin_cycle_relation: relacion de ciclos entre Simics y FSIN, los ciclos que correra FSIN, por defecto es 10
 * long simics_cycle_relation: relacion de ciclos entre Simics y FSIN, los ciclos que correra Simics, por defecto es 1000
 * long packet_size_in_phits: tamao en phits del paquete, un phit son 4 bytes (configurable), por defecto es 32, es pkt_len
 * long serv_addr: puerto de escucha de nuevas conexiones de SIMICS hosts, por defecto es 8082
 * long num_periodos_espera: retardo de interconexion que se introducira en el hosts destino. 
 *							Medido en ciclos SIMICS, por defecto es 0 
 */
void init_exd(long fsin_cycles, long simics_cycles, long packet_size,
			  long simics_hosts_port, long periodos){

	struct sigaction new_action, old_action;
	int error = 0, i = 0, yes = 1;
	struct sockaddr_in server, central_server;


	/* Comprobacion de que los parametros pasados son validos */
	/* fsin_cycle_relation es el numero de ciclos que se ejecutara fsin cada timestamp.*/
	/* simics_cycle_run es el numero de ciclos que se ejecutara Simics cada timestamp.*/
	/* Cuando llega un paquete de Clear_fsin_stats se pone la variable simics_cycle_run*/
	/* al valor verdadero que ha leido de la configuracion, es decir a simics_cycle_relation*/
	/* Esto sirve para que la inicializacion de las simulaciones sea r�ida. */
	fsin_cycle_relation = FSIN_CYCLE_RUN_DEFAULT;
	if (fsin_cycles == 0) {
		fsin_cycle_relation = 10;
	}
	else {
		if (fsin_cycles < 0) {
			perror("valor negativo de fsin_cycle_relation");
			exit (-1);
		}
		else {
			fsin_cycle_relation = fsin_cycles;
		}
	}
	fsin_cycle_run = fsin_cycle_relation;
	simics_cycle_run = SIMICS_CYCLE_RUN_DEFAULT;
	if (simics_cycles == 0) {
		simics_cycle_relation = 10;
	}
	else {
		if (simics_cycles < 0) {
			perror("valor negativo de fsin_cycle_relation");
			exit (-1);
		}
		else {
			simics_cycle_relation = simics_cycles;
		}
	}
	if (simics_hosts_port == 0) {
		serv_addr = 8082;
	}
	else {
		if (simics_hosts_port < 0) {
			perror("valor negativo de serv_addr");
			exit (-1);
		}
		else {
			serv_addr = simics_hosts_port;
		}
	}
	if (periodos == 0) {
		num_periodos_espera = 0;
	}
	else {
		if (periodos < 0) {
			perror("valor negativo de num_periodos_espera");
			exit (-1);
		}
		else {
			num_periodos_espera = periodos;
			printf("periodos %d\n", num_periodos_espera);
		}
	}
	when = num_periodos_espera;
	num_bits_packet_sequence = ceil(log((double)1500/(packet_size_in_phits*phit_size))/log (2));
	mask_eth_id_packet_sequence = 0;
	for (i=0; i<num_bits_packet_sequence; i++){
		mask_eth_id_packet_sequence = (mask_eth_id_packet_sequence << 1) | 1;
	}
    max_id_ethernet_frame = pow (2, sizeof(unsigned long) * 8 - num_bits_packet_sequence) -1;
	mask_eth_id_ethernet_frame = ~0 & ~mask_eth_id_packet_sequence;
	if(DEBUG>=5){
        	printf("num_bits_packet_sequence %d\n", num_bits_packet_sequence);
        	printf ("mask_eth_id_packet_sequence: %08x\n", mask_eth_id_packet_sequence);
		printf("max_id_ethernet_frame %08x\n", max_id_ethernet_frame);
        	//if (max_id_ethernet_frame == 268435455) printf("true\n");
        	//else printf("false\n");
        	printf ("mask_eth_id_ethernet_frame: %08x\n", mask_eth_id_ethernet_frame);
	}

	/* Poner las seales que se encargaran de cerrar sockets y liberar las listas de conexiones y paquetes */
	sigemptyset(&(new_action.sa_mask));
	sigaddset(&(new_action.sa_mask),SIGPIPE);
	sigaddset(&(new_action.sa_mask),SIGINT);
	sigaddset(&(new_action.sa_mask),SIGTERM);
	sigaddset(&(new_action.sa_mask),SIGHUP);
	new_action.sa_handler = signal_handler;
	new_action.sa_flags = 0;
	sigaction(SIGPIPE,&new_action, &old_action);
	sigaction(SIGINT,&new_action, &old_action);
	sigaction(SIGTERM,&new_action, &old_action);
	sigaction(SIGHUP,&new_action, &old_action);

	/* Crear thread de escuchar entrada estandar */

	/* Creamos el thread.
	* En idHilo nos devuelve un identificador para el nuevo thread,
	* Pasamos atributos del nuevo thread NULL para que los coja por defecto
	* Pasamos la funcion que se ejecutara en el nuevo hilo
	* Pasamos un puntero a struct receiving_thread_params como parametro para esa funcion. */
	error = pthread_create (&idHilo, NULL, input_reading_thread, NULL);

	/* Comprobamos el error al arrancar el thread */
	if (error != 0){
		perror ("No puedo crear thread");
		exit (-1);
	}

	/* Crear la lista de paquetes vacia */
	list_packets = CreateVoidList();

	/* Apertura del socket de escucha de nuevas conexiones de Simics hosts*/
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("No hay socket de escucha");
		exit(-2);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(serv_addr);
	if (bind(sock, (struct sockaddr *)&server,
		sizeof server ) < 0) {
			perror("Direccion no asignada al socket de escucha de nuevas conexiones");
			exit(-3);
	}
	listen(sock, 1);


	/* create what looks like an ordinary UDP socket */
     	if ((syn_sock = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
	  perror("syn_sock");
	  close(syn_sock);
	  return;
     	}
        /* set up sender address */
        memset(&syn_addr_s, 0, sizeof(syn_addr_s));
        syn_addr_s.sin_family=AF_INET;
        syn_addr_s.sin_addr.s_addr=inet_addr(FSIN_GROUP); /* differs from receiver */
        syn_addr_s.sin_port=htons(FSIN_PORT);

        /* set up destination address */
        memset(&syn_addr_r, 0, sizeof(syn_addr_r));
        syn_addr_r.sin_family=AF_INET;
        syn_addr_r.sin_addr.s_addr=htonl(INADDR_ANY); /* differs from sender */
        syn_addr_r.sin_port=htons(FSIN_PORT);
	
	/* allow multiple sockets to use the same PORT number */
    	if (setsockopt(syn_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
       	  perror("Reusing ADDR failed in syn_sock");
	  close(syn_sock);
          return;
        }

        /* bind to receive address */
        if (bind(syn_sock, (struct sockaddr *) &syn_addr_r, sizeof(syn_addr_r)) < 0) {
	  perror("bind socket in syn_sock");
	  close(syn_sock);
	  return;
        }
	yes = 1;
	/* The data we send must loop back to the same host its originated */
	if (setsockopt(syn_sock, IPPROTO_IP, IP_MULTICAST_LOOP, &yes, sizeof(yes)) < 0) {
		perror("setsockopt syn_sock IP_MULTICAST_LOOP");
	  	close(syn_sock);
	  	return;
        }

	/* The TTL used must be sufficient to reach several clusters */
	yes = 1;
	if (setsockopt(syn_sock, IPPROTO_IP, IP_MULTICAST_TTL, &yes, sizeof(yes)) < 0) {
		perror("setsockopt syn_sock IP_MULTICAST_TTL");
	  	close(syn_sock);
	  	return;
        }

        /* use setsockopt() to request that the kernel join a multicast group */
        mreq.imr_multiaddr.s_addr=inet_addr(FSIN_GROUP);
        mreq.imr_interface.s_addr=htonl(INADDR_ANY);
        if (setsockopt(syn_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
	  perror("setsockopt syn_sock");
	  close(syn_sock);
	  return;
        }

	/* Meter ambos sockets de escucha de conexiones en el conjunto de sockets (fd_set) */
	/* por los que se escuchan los paquetes que llegan */
	FD_ZERO(&lista_sockets);
	FD_SET(sock, &lista_sockets);
	FD_SET(syn_sock, &lista_sockets);

	/* Para imprimir las estadisticas bien */
	reseted = -1;

}

/**
 * Run the simulation in Execution driven mode.
 *
 * Main function for execution driven simulation using SIMICS.
 */
void run_network_exd(void) {

	int leidos, nuevo_usuario, lector;
	int dispuestos, i;
	int acum =0, len = 0;
	long current_num_hosts_per_Simics = 0;
	char StationAddress[6];
	char msgbuf[32];
	fd_set lb;
	struct timeval tv;

 	char * buf;
	unsigned short count = 0;
	int acum_or_error = 0;
	char * pointer;
	struct arp_record * aux, * aux_recorrido;
	struct packet * pack;

	len = sizeof(syn_addr_r);
	tv.tv_sec = TV_SEC;
    	tv.tv_usec = 0;

  	while (1) {
    	lb = lista_sockets;
    	dispuestos = select(FD_SETSIZE, &lb, (fd_set *)NULL, (fd_set *) NULL, &tv);
		if (dispuestos < 0) {
			perror("select ha fallado");
			continue;	
		}
		if (dispuestos == 0){
			//Puede que se haya perdido un CONTINUE lo remandamos con un timestamp antiguo
			if (num_Simics_hosts != nprocs){
			//Todavia no se han conectado todos los hosts
				tv.tv_sec = TV_SEC;
    				tv.tv_usec = 0;
				continue;
			}
			printf("Puede que se haya perdido un CONTINUE lo remandamos con un timestamp antiguo\n");
 			//escribe el Go Ahead GA en Simics Central
			memset(msgbuf, 0, sizeof(msgbuf));
			strcat(msgbuf, "CONTINUE:");
			memcpy(msgbuf + strlen("CONTINUE:"), &timestamp_old, sizeof(timestamp_old));
			memcpy(msgbuf + strlen("CONTINUE:") + sizeof(timestamp_old), &simics_cycle_run, sizeof(simics_cycle_run));
			if (sendto(syn_sock, &msgbuf, strlen("CONTINUE:") + sizeof(timestamp_old) + sizeof(simics_cycle_run),
				0, (struct sockaddr *) &syn_addr_s, sizeof(syn_addr_s)) < 0) {
	  			perror("sendto syn_sock CONTINUE");
	  			close(syn_sock);
	  			return;
			}
			//Se pone 10 seg. en el tiempo que espera el select para que si se pierde un
			//CONTINUE: se reenvie el CONTINUE: y se recupere la simulacion de la perdida
			tv.tv_sec = TV_SEC;
    			tv.tv_usec = 0;
			continue;
		}


		if (FD_ISSET(sock, &lb)) {//nueva conexion Simics
			nuevo_usuario = accept(sock, NULL, NULL);
			aux = (struct arp_record *)malloc (sizeof(struct arp_record));
		  	if (recv(nuevo_usuario, aux->StationAddress, sizeof(aux->StationAddress), 0) != sizeof(aux->StationAddress)){
				perror("error al recibir el StationAddress");
				close(nuevo_usuario);
				continue;
			}
			aux->socket = nuevo_usuario;
			aux->paquetes = 0;
			aux->FSIN_node_number = (unsigned long) aux->StationAddress[sizeof(aux->StationAddress) - 1];
			/*aux->FSIN_node_number = FSIN_next_node_number;
			FSIN_next_node_number = (FSIN_next_node_number + 1) % LONG_MAX;*/

			gettimeofday(&aux->tiempo, 0);
			if (!lista) {
				lista = CreateVoidList();
			}
			else {
				//ver si ya estaba en la lista de conexiones y si esta borrarla y darla de alta de nuevo
				aux_recorrido = (struct arp_record *) StartLoop(lista);
				for (; aux_recorrido; aux_recorrido = (struct arp_record *) GetNext(lista)){
					if(!memcmp(aux->StationAddress, aux_recorrido->StationAddress, 6)){
						/* El nmero de nodo sigue siendo el mismo, pornerlo y corregir */
						/* la variable FSIN_next_node_number */
						aux->FSIN_node_number = aux_recorrido->FSIN_node_number;
						/*FSIN_next_node_number --;
						if (FSIN_next_node_number < 0){
							printf("FSIN_next_node_number < 0, es %d\n", FSIN_next_node_number);
							FSIN_next_node_number = 0;
						}*/
						close(aux_recorrido->socket);
						printf("se ha quitado de la lista un socket abierto:%d\n", aux_recorrido->socket);
						RemoveFromList(lista, (void *) aux_recorrido);
						free(aux_recorrido);
					}
				}
			}
			AddLast(lista, (void *) aux);
			FD_SET(nuevo_usuario, &lista_sockets);
			write(nuevo_usuario, "OK", sizeof("OK"));
			printf("se ha conectado en socket %d: ", nuevo_usuario);
			for (i=0; i<6; i++){
				printf("%02x ", aux->StationAddress[i]&0xff);
			}
			printf(", nodo FSIN: %d\n", aux->FSIN_node_number);
		}
		else if (FD_ISSET(syn_sock, &lb)) {
			//nueva conexion Simics para sincronizacion
			leidos = 0;
			memset(msgbuf, 0, sizeof(msgbuf));
			if ((leidos=recvfrom(syn_sock, msgbuf, sizeof(msgbuf), 0, 
				(struct sockaddr *) &syn_addr_r, &len)) < 0) {
	  	  		perror("recvfrom syn_sock CONNECT");
		  		close(syn_sock);
	  	  		return;
			}
			
			if (DEBUG >= 10) printf("%s\n", msgbuf);
			if (DEBUG >= 10) printf("leidos:%d\n", leidos);
			if(!strncmp(msgbuf, "TS:", strlen("TS:"))){
				while(leidos < strlen("TS:") + sizeof(StationAddress) + strlen(":")+ sizeof(timestamp)){
					if ((leidos=recvfrom(syn_sock, msgbuf, sizeof(msgbuf), 0, 
						(struct sockaddr *) &syn_addr_r, &len)) < 0) {
	  	  				perror("recvfrom syn_sock CONNECT");
		  				close(syn_sock);
	  	  				return;
					}
				}
				memcpy(StationAddress, msgbuf + strlen("TS:"), sizeof(StationAddress));
				memcpy(&timestamp, msgbuf + strlen("TS:") + sizeof(StationAddress) + strlen(":"), sizeof(timestamp));
				
				if (timestamp_old > timestamp) {
					printf("error recibiendo timestamp: %llu timestamp_old:%llu\n", timestamp, timestamp_old);
				}
				if (!timestamp_init) timestamp_init = timestamp;
				timestamp_end = timestamp;
				
				if (DEBUG >= 10)
					printf("timestamp: %llu timestamp_old:%llu num_Simics_hosts %d \n", 
					timestamp, timestamp_old, num_Simics_hosts);
				if (AddSync(StationAddress, sizeof(StationAddress), timestamp) && num_Simics_hosts == nprocs){
				//All Simics hosts are connected and have sent their timestamps
					timestamp_old = timestamp;
					packet_dispatcher(lista, list_packets);
					//escribe el Go Ahead GA en Simics Central
					memset(msgbuf, 0, sizeof(msgbuf));
					strcat(msgbuf, "CONTINUE:");
					memcpy(msgbuf + strlen("CONTINUE:"), &timestamp, sizeof(timestamp));
					memcpy(msgbuf + strlen("CONTINUE:") + sizeof(timestamp), &simics_cycle_run, sizeof(simics_cycle_run));
					if (sendto(syn_sock, &msgbuf, strlen("CONTINUE:") + sizeof(timestamp)+ sizeof(simics_cycle_run), 
						0, (struct sockaddr *) &syn_addr_s, sizeof(syn_addr_s)) < 0) {
	  					perror("sendto syn_sock CONTINUE");
	  					close(syn_sock);
	  					return;
					}
					//Se pone TV_SEC seg. en el tiempo que espera el select para que si se pierde un
					//CONTINUE: se reenvie el CONTINUE: y se recupere la simulacion de la perdida
					tv.tv_sec = TV_SEC;
    					tv.tv_usec = 0;
				}
				continue;
			}
			else if(!strncmp(msgbuf, "CONNECT:", strlen("CONNECT:"))) {	
				while(leidos < strlen("CONNECT:") + sizeof(current_num_hosts_per_Simics) + strlen(":") + sizeof(StationAddress)){
					if ((leidos=recvfrom(syn_sock, msgbuf, sizeof(msgbuf), 0, 
						(struct sockaddr *) &syn_addr_r, &len)) < 0) {
	  	  				perror("recvfrom syn_sock CONNECT");
		  				close(syn_sock);
	  	  				return;
					}
				}
				
				if (strncmp("CONNECT:", msgbuf, strlen("CONNECT:"))) continue;

				memcpy(&current_num_hosts_per_Simics, msgbuf + strlen("CONNECT:"), sizeof(current_num_hosts_per_Simics));
				memcpy(StationAddress, msgbuf + strlen("CONNECT:") + sizeof(current_num_hosts_per_Simics) + strlen(":"), 
					sizeof(StationAddress));
				
				CreateSync(&StationAddress, sizeof(StationAddress));
				
				num_Simics_hosts += current_num_hosts_per_Simics;
				assert(num_Simics_hosts <= nprocs);
				
				printf("CONNECT:");
				for (i=0; i<6; i++){
					printf("%02x ", StationAddress[i]&0xff);
				}
				printf("\n");
				
				memset(msgbuf, 0, sizeof(msgbuf));
				memcpy(msgbuf, "OK:", strlen("OK:"));
				memcpy(msgbuf + strlen("OK:"), StationAddress, sizeof(StationAddress));
				if (sendto(syn_sock, &msgbuf, strlen("OK:") + sizeof(StationAddress), 0, (struct sockaddr *) &syn_addr_s,
					sizeof(syn_addr_s)) < 0) {
	  				perror("sendto syn_sock OK");
	  				close(syn_sock);
				}
			}
		}
		else {
			for (escritor=0; escritor<FD_SETSIZE; escritor++) {
				if(FD_ISSET(escritor, &lb)) {
					leidos = 0;
					while (leidos != sizeof(count)){
						//lee el tamao del buffer
						acum_or_error = read(escritor, (void *) ((&count) + leidos), sizeof(count) - leidos);
						//printf ("count:%d\n", count);
						if (acum_or_error <= 0) {
							/*
							aux = (struct arp_record *) StartLoop(lista);
							for (; aux; aux = (struct arp_record *) GetNext(lista)){
								if( aux->socket == escritor){
									RemoveFromList(lista, (void *) aux);
									free(aux);
									if (ElementsInList(lista) == 0) DestroyList(&lista);
									break;
								}
							}
							if (DEBUG >=0)
								printf("Cerramos el socket al leer el numero de bytes del paquete leidos:%d!=%d\n",
												leidos, sizeof(count));
							close(escritor);
							FD_CLR(escritor, &lista_sockets);
							*/
							
							aux = (struct arp_record *) StartLoop(lista);
							for (; aux; aux = (struct arp_record *) GetNext(lista)){
								if( aux->socket == escritor){
									for (i=0; i<6; i++){
										printf("%02x ", aux->StationAddress[i]&0xff);
									}
									printf(":");
									printf("Error en numero de bytes del mensaje, leidos:%d!=%d\n",
										leidos, sizeof(count));
									for (i=0; i<6; i++){
										printf("%02x ", aux->StationAddress[i]&0xff);
									}
									printf(":");
									printf("resultado del read: acum_or_error=%d\n", acum_or_error);
									break;
								}
							}
							leidos = -1;
							break;
						}
						leidos += acum_or_error;
					}
					if (leidos == -1) break;
					if (DEBUG>=5){
						if (count != 0) {
							printf ("buffer de longitud: %d\n", count);
						}
						else {
							acum ++;
							if (!(acum = acum % 1000)) printf ("%d", count);
							continue;
						}
					}
					leidos = 0;
					buf = (char *) malloc ((count + sizeof(count) + sizeof(when)) * sizeof(char));
					// Rellenar los campos a�didos del buffer
					memcpy(buf, (void *)&when, sizeof(when));
					memcpy(buf + sizeof(when), (void *)&count, sizeof(count));

					while (leidos != count){
						//lee el buffer, en el que hemos dejado el hueco del tamano
						//del numero de ciclos de espera en la cola de inyeccion
						//(num_periodos_espera) y el tamano del buffer
						acum_or_error = read(escritor, buf + sizeof(when) + sizeof(count) + leidos, count - leidos);
						//leidos = read(escritor, buf, sizeof(buf));
						if (acum_or_error <= 0) {
							aux = (struct arp_record *) StartLoop(lista);
							for (; aux; aux = (struct arp_record *) GetNext(lista)){
								if( aux->socket == escritor){
									RemoveFromList(lista, (void *) aux);
									free(aux);
									if (ElementsInList(lista) == 0) DestroyList(&lista);
									break;
								}
							}
							if (DEBUG >=0) printf("Ha habido un cierre del socket al leerlo leidos <=0\n");
							close(escritor);
							FD_CLR(escritor, &lista_sockets);
							break;
						}
						leidos += acum_or_error;
					}
					//se ha leido un paquete
					if (DEBUG >=9) {
						printf("Ha leido en el paquete: %d bytes del socket:%d\n", leidos, escritor);
						for (i=0; i<16; i++){
							printf("%02x ", buf[i + sizeof(when) + sizeof(count)]&0xff);
						}
						printf("\n");
					}
					pack = (struct packet *)malloc (sizeof(struct packet));
					pack->buffer = buf;
					pack->length = count;
					pack->socket = escritor;
					pack->timestamp = timestamp;
					pack->id_ethernet_frame = id_next_ethernet_frame;
					id_next_ethernet_frame = (id_next_ethernet_frame + 1) % max_id_ethernet_frame;
					pack->num_packets_to_come = 0;
					pack->FSIN_packets = NULL;
					pack->num_FSIN_packets = 0;
					AddLast(list_packets, (void *) pack);
					if (!run_FSIN) packet_dispatcher(lista, list_packets);
					if (DEBUG >=5) printf("run_network_exd: trama %d, encoladas: %d\n", pack->id_ethernet_frame, ElementsInList(list_packets));
					break;
				}
			}
		}
	}
}


// Auxiliary functions to get FSIN STATS when a special ICMP packet reach
void die(char* str){
	fprintf(stderr,"%s\n",str);
	exit(1);
}

void get_ip_addr(struct in_addr* in_addr,char* str){

struct hostent *hostp;

in_addr->s_addr=inet_addr(str);
if(in_addr->s_addr == -1){
        if( (hostp = gethostbyname(str)))
                bcopy(hostp->h_addr,in_addr,hostp->h_length);
        else {
                fprintf(stderr,"send_arp: unknown host %s\n",str);
                exit(1);
	}
}
}

void get_hw_addr(u_char* buf, char* str){

int i;
char c,val;

for(i=0;i<ETH_ALEN;i++){
        if( !(c = tolower(*str++))) die("Invalid hardware address");
        if(isdigit(c)) val = c-'0';
        else if(c >= 'a' && c <= 'f') val = c-'a'+10;
        else die("Invalid hardware address");

        *buf = val << 4;
        if( !(c = tolower(*str++))) die("Invalid hardware address");
        if(isdigit(c)) val = c-'0';
        else if(c >= 'a' && c <= 'f') val = c-'a'+10;
        else die("Invalid hardware address");

        *buf++ |= val;

        if(*str == ':')str++;
	}
}

int CreateSync(char * StationAddress, int len){
struct sync * new_sync = NULL;
/* Crear la lista de sincronizaci� vac� */
if (!list_sync)	list_sync = CreateVoidList();
new_sync = (struct sync *)malloc (sizeof(struct sync));
if (!new_sync) return 0;
memcpy(new_sync->StationAddress, StationAddress, len);
new_sync->timestamp = 0;
AddLast(list_sync, (void *)new_sync);
return 1;
}

int AddSync(char * StationAddress, int len, unsigned long long timestamp){
int send = 1;
struct sync * aux;

aux = (struct sync *) StartLoop(list_sync);
for (; aux; aux = (struct sync *) GetNext(list_sync)){
	if(!memcmp(aux->StationAddress, StationAddress, len)){
		aux->timestamp = timestamp;
	}
	else if (aux->timestamp != timestamp) send = 0;
}
return send;
}

void SwapBytes (void * b1, void * b2, unsigned short len){
void * aux;

aux = malloc (len * sizeof(char));
memcpy(aux, b1, len);
memcpy(b1, b2, len);
memcpy(b2, aux, len);
free(aux);
}

/* this function generates header checksums */
unsigned short csum (unsigned short *buf, int nwords){
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

#endif
