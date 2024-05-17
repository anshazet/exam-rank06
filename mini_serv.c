#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

void putstr(char *str, int fd)
{
	write(fd, str, strlen(str));
	exit(1);
}

void send_all(char *buffer, int server_fd, int client_fd, int fd_size)
{
	for (int i = 0; i <= fd_size; i++)
		if (i != server_fd && i != client_fd)
			send(i, buffer, strlen(buffer), 0);
}

int main(int argc, char **argv)
{
	int server_fd, client_fd, size_fd, readed, db[65535] = {0}, limit = 0;
	struct sockaddr_in server_addr;
	fd_set old_fd, new_fd;
	char buffer[200000], buffer2[150000];

	if (argc != 2)
		putstr("Wrong number of arguments\n", 2);
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		putstr("Fatal error\n", 2);
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(2130706433);
	server_addr.sin_port = htons(atoi(argv[1]));
	if ((bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr))) != 0)
		putstr("Fatal error\n", 2);
	if (listen(server_fd, 10) != 0)
		putstr("Fatal error\n", 2);
	FD_ZERO(&old_fd);
	FD_ZERO(&new_fd);
	FD_SET(server_fd, &new_fd);
	size_fd = server_fd;
	while (1)
	{
		old_fd = new_fd;
		if (select(size_fd + 1, &old_fd, NULL, NULL, NULL) < 0)
			putstr("Fatal error\n", 2);
		for (int id = 0; id <= size_fd; id++)
		{
			if (FD_ISSET(id, &old_fd) == 0)
				continue;
			if (id == server_fd)
			{
				if ((client_fd = accept(server_fd, NULL, NULL)) < 0)
					putstr("Fatal error\n", 2);
				if (client_fd > size_fd)
					size_fd = client_fd;
				db[client_fd] = limit++;
				FD_SET(client_fd, &new_fd);
				sprintf(buffer, "server: client %d just arrived\n", db[client_fd]);
				sendall(buffer, server_fd, client_fd, size_fd);
			}
			else
			{
				bzero(buffer, 200000);
				bzero(buffer2, 150000);
				readed = 1;
				while (readed == 1 && (!buffer2[0] || buffer2[strlen(buffer2) - 1] != '\n'))
					readed = recv(id, &buffer2[strlen(buffer2)], 1, 0);
				if (readed <= 0)
				{
					sprintf(buffer, "server: client %d just left\n", db[id]);
					sendall(buffer, server_fd, id, size_fd);
					FD_CLR(id, &new_fd);
					close(id);
				}
				else
				{
					sprintf(buffer, "client %d: %s", db[id], buffer2);
					sendall(buffer, server_fd, id, size_fd);
				}
			}
		}
	}
	return (0);
}