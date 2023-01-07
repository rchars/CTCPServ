#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>


#define BUFF_SIZE 1024


int main(void) {
    int serv_sock_d = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9999);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bind(serv_sock_d, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(serv_sock_d, 0);

    ssize_t readed_bytes;
    long int cli_msg_len = 0;
    int cli_sock_d, end_index = 0, start_index = 0, eol_index = -1;
    char buff[BUFF_SIZE];
    char *cli_msg = NULL, *eol_found;
    char serv_msg[] = "serv msg\n";
    bool cli_connected = false, end_loop = false;
    while(true) {
        if(!cli_connected) {
            end_index = 0;
            eol_index = -1;
            start_index = 0;
            cli_msg_len = 0;
            // cli_msg = (char *)realloc(cli_msg, 0);
            cli_msg = NULL;
            cli_sock_d = accept(serv_sock_d, NULL, NULL);
            cli_connected = true;
        }
        end_loop = false;
        while(!end_loop) {
            if(end_index == 0) {
                readed_bytes = read(cli_sock_d, buff, BUFF_SIZE);
                if(readed_bytes <= 0) {
                    cli_connected = false;
                    break;
                }
                else {
                    end_index = readed_bytes - 1;
                }
            }
            eol_found = strchr(buff + start_index, '\n');
            int copy_size;
            if(eol_found == NULL) {
                end_loop = false;
                copy_size = end_index - start_index + 1;
            }
            else {
                end_loop = true;
                eol_index = eol_found - buff;
                copy_size = eol_index - start_index + 1;
            }
            cli_msg = (char *)realloc(cli_msg, cli_msg_len + copy_size);
            memcpy(cli_msg + cli_msg_len, buff + start_index, copy_size);
            cli_msg_len += copy_size;
            if(eol_index == end_index || eol_found == NULL) {
                start_index = 0;
                end_index = 0;
            }
            else {
                start_index = eol_index + 1;
            }
        }
        if(readed_bytes > 0) {
            cli_msg = (char *)realloc(cli_msg, cli_msg_len + 1);
            cli_msg[cli_msg_len] = '\0';
            fprintf(stdout, "[%ld] => %s", cli_msg_len, cli_msg);
            send(cli_sock_d, serv_msg, strlen(serv_msg), 0);
            cli_msg = NULL;
            cli_msg_len = 0;
        }
    }
    close(serv_sock_d);
    free(cli_msg);
    return EXIT_SUCCESS;
}
