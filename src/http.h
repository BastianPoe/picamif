#ifndef _HTTP_H
#define _HTTP_H

int send_response(struct MHD_Connection *connection, int code, char *data);
int bad_request(struct MHD_Connection *connection, char *reason);
int internal_server_error(struct MHD_Connection *connection, char *reason);
int not_found(struct MHD_Connection *connection);
int send_response_bin(struct MHD_Connection *connection, int code, char *contentType, unsigned long filesize, char *data);

#endif
