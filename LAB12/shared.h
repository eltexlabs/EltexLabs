//// 25.10.19
//// LAB #12 - shared header
////
//// Student: Alexey Leushin
////



// Includes //
#include <stdint.h>		// uint32_t and others
#include <stdbool.h>	// bool


// Definitions and typedefs //
#define FAIL -1
#define COND_EXIT(COND, EXMSG) { if (COND) { printf("=== Error in %s() ===\n%s\n", __func__, EXMSG); exit(EXIT_FAILURE); } }
#define COND_MSG(COND, MSG) { if (COND) { printf("=== Warning in %s() ===\n%s\n", __func__, MSG); } }
//#define DEBUG

#define CL_TRYCOUNT 10	// Client number of connect attempts
#define SHD_RD	0
#define SHD_WR	1
#define SHD_ALL	2
#define BUF_SZ		120
#define SV_MSG_QSZ	10	// Server messages queue size
#define SV_SOCK_QSZ	5	// Server socket queue size
#define MAX_SLEEP	3	// Max sleep time

#define NUM_STRINGS		10
#define STRINGS			"quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog", "doge", "bla-bla-bla"

#define MSG_ECHO1		"udp1"
#define MSG_ECHO2		"udp2"
#define MSG_AUTH1		"tcp1"
#define MSG_AUTH2		"tcp2"
#define MSG_AUTH_ACK	"okay"
#define MSG_AUTH_DENY	"nope"


// Func prototypes
extern void PrepInetClientSock(const char * address, int port, int * sock, void * sockaddr, bool udp);
extern void PrepInetServerSock(const char * address, int port, int * sock, void * sockaddr, bool udp, int qsize);
extern char * AllocString(const char * str);
extern void FreeString(char * str);

extern int UserQuit();


// Message types
/*
#pragma pack(1)
typedef enum msgtype_t
{
	MSG_TEXT,
	MSG_SV_ECHO1,
	MSG_SV_ECHO2,
} msgtype_t;

// Client types
#pragma pack(1)
typedef enum msggroup_t
{
	CL_EMITTER,
	CL_CONSUMER,
} msggroup_t;


// Base message
#pragma pack(1)
typedef struct msg_base_t
{
	uint8_t sign[4];	// Signature: 'LB12'
	uint16_t ver;		// Version: 1
	msgtype_t type;		// Message type
	uint32_t datasz;	// Data size
	int8_t data[];		// Data
} msg_base_t;

// Text message
#pragma pack(1)
typedef struct msg_text_t
{
	msg_base_t head;	// Base: header
	char text[];		// Data: text
} msg_text_t;

// Echo message
#pragma pack(1)
typedef struct msg_echo_t
{
	msg_base_t head;	// Base: header
	msggroup_t group;	// Data: group
} msg_echo_t;
*/
