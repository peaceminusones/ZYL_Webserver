#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//下载目标文件 : http://ftp.gnu.org/gnu/bc/bc-1.03.tar.gz
#define HOST_ADDR "ftp.gnu.org"
#define FILE_PATH "/gnu/bc/bc-1.03.tar.gz"
#define FILE_NAME "bc-1.03.tar.gz"

void write_to_socket(int socketfd, const char *str)
{
    write(socketfd, str, strlen(str));
}

int main(void)
{
    char *pstr = NULL;
    int size = 0;
    int sum = 0;
    unsigned long filesize = 0;
    struct sockaddr_in sk_addr;
    struct hostent *phost = NULL;
    char buf[1024] = {0};
    float processbar = 0.0f;

    // create TCP socket
    int sk_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sk_fd < 0)
    {
        perror("socket failed");
        exit(1);
    }

    phost = gethostbyname(HOST_ADDR);

    memset(&sk_addr, 0x00, sizeof(sk_addr));
    sk_addr.sin_family = AF_INET;
    sk_addr.sin_port = htons(80);
    sk_addr.sin_addr.s_addr = ((struct in_addr *)phost->h_addr)->s_addr;

    int connfd = connect(sk_fd, (struct sockaddr *)&sk_addr, sizeof(sk_addr));
    if (connfd < 0)
    {
        perror("connect failed");
        close(sk_fd);
        exit(1);
    }

    write_to_socket(sk_fd, "GET " FILE_PATH " HTTP/1.1\r\n");
    write_to_socket(sk_fd, "Accept: */*\r\n");
    write_to_socket(sk_fd, "Host: " HOST_ADDR "\r\n");
    write_to_socket(sk_fd, "Connection: Keep-Alive\r\n");
    write_to_socket(sk_fd, "\r\n");
    shutdown(sk_fd, SHUT_WR);

    FILE *fp = fdopen(sk_fd, "r");
    FILE *newfp = fopen(FILE_NAME, "w+");
    memset(buf, 0x00, sizeof(buf));
    while (fgets(buf, 1023, fp))
    {
        printf("%s\n", buf);

        if (filesize != 0) /* find CRLF */
        {
            if (2 == strlen(buf))
            {
                break;
            }
        }
        else /* get file size */
        {
            pstr = strstr(buf, "Content-Length:");
            if (pstr != NULL)
            {
                pstr = strchr(pstr, ':');
                pstr++;
                filesize = strtoul(pstr, NULL, 10);
                printf("filesize = %lu\n", filesize);
            }
        }
        memset(buf, 0x00, sizeof(buf));
    }

    while (1)
    {
        size = fread(buf, 1, 1024, fp);
        sum += size;
        if (0 == size)
        {
            break;
        }
        size = fwrite(buf, 1, size, newfp);
        processbar = (float)sum / (float)filesize;
        printf("%6d/%lu[%2.1f]\n", sum, filesize, processbar * 100);
    }
    fclose(fp);
    fclose(newfp);

    return 0;
}