#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include "http.h"
#include "strings.h"

int send_response(struct MHD_Connection *connection, int code, char *data) {
    struct MHD_Response *response;
    int ret;
    response = MHD_create_response_from_data (strlen (data), (void*) data, MHD_NO, MHD_YES);
    ret = MHD_queue_response (connection, code, response);
    MHD_destroy_response (response);
    return ret;
}

int send_response_bin(struct MHD_Connection *connection, int code, char *contentType, unsigned long filesize, char *data) {
    struct MHD_Response *response;
    int ret;
    response = MHD_create_response_from_data (filesize, (void*) data, MHD_NO, MHD_YES);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, contentType);
    ret = MHD_queue_response (connection, code, response);
    MHD_destroy_response (response);
    return ret;
}

int internal_server_error(struct MHD_Connection *connection, char *reason) {
    char *data = NULL;
    vstrdupcat(&data, "<html><body><h1>Error</h1><h2>Internal Server Error</h2><p>", reason, "</p></body></html>", NULL);
    int ret = send_response(connection, 500, data);
    free(data);
    return ret;
}

int bad_request(struct MHD_Connection *connection, char *reason) {
    char *data = NULL;
    vstrdupcat(&data, "<html><body><h1>Error</h1><h2>Bad Request</h2><p>", reason, "</p></body></html>", NULL);
    int ret = send_response(connection, 400, data);
    free(data);
    return ret;
}

int not_found(struct MHD_Connection *connection) {
    return send_response(connection, 404, "<html><body><h1>Error</h1><h2>Not Found</h2><p>The requested resource was not found on this server.</p></body></html>");
}
