
typedef struct
{
  TCP_SOCK sock;
  UREG state;
  UINT8 header_pos;
  UINT16 block_length;
  UINT16 metadata_interval;
  char req[32];
}GET_SOCK;
extern GET_SOCK get_sock;
extern void StartGET(void);
extern volatile UINT8 GET_WINDOW_STATE;

enum GET_WINDOW_STATES
{
  GET_WINDOW_STATE_UPDATE=0,
  GET_WINDOW_STATE_ZERO
};