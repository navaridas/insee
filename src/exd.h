/**
* @file
* @brief	Execution driven simulation & Interface with Simics needed definitions.
*/


#if (EXECUTION_DRIVEN != 0)
#include <sys/time.h>
#include <pthread.h>
#include <netinet/in.h>

#include "globals.h"
#include "list.h"


#define DEBUG 2

/* FSIN statistics collection */
#define CLEAR_STATS 1
#define GET_STATS 2
#define TARG_IP_ADDR "1.2.3.4"
#define TARG_HW_ADDR "11:22:33:44:55:66"
#define IP_ADDR_LEN 4

/* FSIN node latency alteration */


char broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
int run_FSIN = 0;
unsigned long long timestamp, timestamp_old = 0, periodo, when = 0;
int sock, escritor, syn_sock;
long num_Simics_hosts = 0;
struct sockaddr_in syn_addr_r, syn_addr_s;
struct ip_mreq mreq;
int SIGPIPE_caught, received_terminate_signal = 0;
pthread_t idHilo;
fd_set lista_sockets;
long FSIN_next_node_number = 0;
long id_next_ethernet_frame = 0;
unsigned long max_id_ethernet_frame;
unsigned short num_bits_packet_sequence;
long mask_eth_id_packet_sequence;
long mask_eth_id_ethernet_frame;
#define PACKET_SIZE_IN_BYTES (packet_size_in_phits * phit_len)
long fsin_cycle_relation;
long simics_cycle_relation;
CLOCK_TYPE fsin_cycle_run; // Ver descripcion en init_exd
long simics_cycle_run; // Ver descripcion en init_exd
#define FSIN_CYCLE_RUN_DEFAULT 10
#define SIMICS_CYCLE_RUN_DEFAULT 100000
#define TV_SEC 10
//long packet_size_in_phits; // Esto es pkt_len
#define FSIN_PORT 12345
#define FSIN_GROUP "225.0.0.37"
long serv_addr;
long num_periodos_espera;
// El porcentaje que determina si un paquete esperara
// num_periodos_espera * periodos en ciclos Simics
#define WAIT_PERCENT 0.00001f
// Tiempos usados para mostrar las estadisticas de FSIN con respecto de Simics
static time_t start_time, end_time;

unsigned long long veces_que_entra_FSIN = 0;
unsigned long long timestamp_init = 0;
unsigned long long timestamp_end = 0;

//Estructura que guarda la informacion de cada conexion relacionada con
//la sincronización guarda MAC y el timestamp que ha enviado
struct sync {
	char StationAddress[6];
	unsigned long long timestamp;
};
list * list_sync = NULL;

//Estructura que guarda la informacion de cada conexion a switch identificada
//por el socket por el que se conecta y la direccion MAC de sus paquetes
struct arp_record {
	char StationAddress[6];
	int socket;
	struct timeval tiempo;
	unsigned long paquetes;
	unsigned long FSIN_node_number;
};
list * lista = NULL;

//Estructura que guarda la informacion de un paquete recibido
struct packet {
	char * buffer;	//el paquete recibido y ocho bytes por el principio donde se pondra el tiempo
	//de espera en la cola de inyeccion de Simics y despues dos bytes por el
	//principio vacios para anadir la longitud del paquete
	unsigned short length;//longitud del paquete en si
	int socket; // por el que ha llegado el paquete
	unsigned long long timestamp; // de SIMICS
	unsigned long id_ethernet_frame; // identificador que se le asigna a la trama ethernet y que es unico
	unsigned short num_packets_to_come; // numero de paquetes que quedan por llegar de la trama
	/* Si se tiran phits en algn momento debido a que se llenan los buffers de inyeccion */
	/* habria que hacer una rutina que cada cierto tiempo limpiase de la cola las tramas */
	/* ethernet que estuviesen pendientes de llegar por mas de un tiempo razonable */

	struct FSIN_packet * FSIN_packets;
	/* Array de estructuras del tipo struct FSIN_packet que representan los paquetes en */
	/* que se divide a una trama ethernet */
	unsigned short num_FSIN_packets; // el numero de paquetes FSIN en que se ha dividido
									 // la trama ethernet
	unsigned long FSIN_node_source;	// El numero de nodo FSIN de donde proviene esta trama ethernet
	unsigned long FSIN_node_destination; // El numero de nodo FSIN a donde se mandaran
										// los paquetes y phits en que se dividira
										// esta trama ethernet

};//8 (periodo * num_periodos_espera), 2 (length), length (buf[ ]) = char * buffer
list * list_packets = NULL;

struct FSIN_packet {
	unsigned short number_phits; // numero de phits en que se ha dividido el paquete de FSIN
	unsigned char tail_arrived;	// indica que el phit que contiene la cola ha llegado
								// a su destino 1 TRUE y 0 FALSE
	unsigned char packet_generated;	// indica que el paquete se ha generado correctamente y no ha tenido que
								// esperar para ser generado debido a que el buffer de inyeccion estaba lleno
								// 1 TRUE y 0 FALSE
};

/* Funcion de inicializacion para la Simulacion conducida por la ejecucion */
/* Parametros: */
/* long fsin_cycle_relation: relacion de ciclos entre Simics y FSIN, los ciclos que correra FSIN, por defecto es 10 */
/* long simics_cycle_relation: relacion de ciclos entre Simics y FSIN, los ciclos que correra Simics, por defecto es 1000 */
/* long packet_size_in_phits: tamaï¿½ en phits del paquete, un phit son 4 bytes (configurable), por defecto es 32, es pkt_len */
/* long serv_addr: puerto de escucha de nuevas conexiones de SIMICS hosts, por defecto es 8082 */
/* long num_periodos_espera: retardo de interconexion que se introducira en el hosts destino. */
/*							Medido en pasos de sincronizacion, por defecto es 0 */
void init_exd(long, long, long, long, long);

/* Funcion que ejecuta el bucle de recepcion de paquetes de los Simics hosts y */
/* timestamps del Central y los inyecta en FSIN */
void run_network_exd(void);

/* Funcion que comprueba si ha llegado un paquete y una trama depues de que haya */
/* llegado un phit a su destino en FSIN */
/* Los parametros son el nodo al que ha llegado el phit y el phit en cuestion */
void SIMICS_phit_away(long, phit);

#endif

