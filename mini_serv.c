#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

void error()
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

void sendall(char *buffer, int server, int client, int size)
{
	for (int i = 0; i <= size; i++)
		if (i != client && i != server)
			send(i, buffer, strlen(buffer), 0);
}

int main(int ac, char **av)
{
	int server, client, size, readed, db[65535] = {0}, limit = 0;
	struct sockaddr_in servaddr;
	fd_set old_fd, new_fd;
	char buffer[450000], buffer2[400000];

	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == -1)
		error();

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1]));

	if ((bind(server, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		error();
	if (listen(server, 10) != 0)
		error();

	FD_ZERO(&old_fd);
	FD_ZERO(&new_fd);
	FD_SET(server, &new_fd);
	size = server;

	while (1)
	{
		old_fd = new_fd;
		if (select(size + 1, &old_fd, NULL, NULL, NULL) < 0)
			error();
		for (int id = 0; id <= size; id++)
		{
			if (FD_ISSET(id, &old_fd) == 0)
				continue;
			if (id == server)
			{
				if ((client = accept(server, NULL, NULL)) < 0)
					error();
				if (client > size)
					size = client;
				db[client] = limit++;
				FD_SET(client, &new_fd);
				sprintf(buffer, "server: client %d just arrived\n", db[client]);
				sendall(buffer, server, client, size);
			}
			else
			{
				bzero(buffer, 450000);
				bzero(buffer2, 400000);
				readed = 1;
				while (readed == 1 && (!buffer2[0] || buffer2[strlen(buffer2) - 1] != '\n'))
					readed = recv(id, &buffer2[strlen(buffer2)], 1, 0);
				if (readed <= 0) // <=
				{
					sprintf(buffer, "server: client %d just left\n", db[id]);
					sendall(buffer, server, id, size);
					FD_CLR(id, &new_fd);
					close(id);
				}
				else
				{
					sprintf(buffer, "client %d: %s", db[id], buffer2);
					sendall(buffer, server, id, size);
				}
			}
		}
	}
	return (0);
}