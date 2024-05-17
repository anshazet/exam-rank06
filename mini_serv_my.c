#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

void ft_error()
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

void send_all(char *buffer, int server, int client, int size)
{
	for (int i = 0; i <= size; i++)
		if (i != client && i != server)
			send(i, buffer, strlen(buffer), 0);
}

int main(int ac, char **av)
{
	int server, client, size, readed, db[65535] = {0}, client_id = 0;
	struct sockaddr_in servaddr;
	fd_set old_fd, new_fd;
	char buffer[400000], buffer2[450000];

	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == -1)
		ft_error();

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1]));

	if ((bind(server, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		ft_error();
	if (listen(server, 10) != 0)
		ft_error();

	FD_ZERO(&old_fd);
	FD_ZERO(&new_fd);
	FD_SET(server, &new_fd);
	size = server;

	while (1)
	{
		old_fd = new_fd;
		if (select(size + 1, &old_fd, NULL, NULL, NULL) < 0)
			ft_error();
		for (int id = 0; id <= size; id++)
		{
			if (FD_ISSET(id, &old_fd) == 0)
				continue;
			if (id == server)
			{
				if ((client = accept(server, NULL, NULL)) < 0)
					ft_error();
				if (client > size)
					size = client;
				db[client] = client_id++;
				FD_SET(client, &new_fd);
				sprintf(buffer, "server: client %d just arrived\n", db[client]);
				send_all(buffer, server, client, size);
			}
			else
			{
				bzero(buffer, 400000);
				bzero(buffer2, 450000);
				readed = 1;
				while (readed == 1 && (!buffer2[0] || buffer2[strlen(buffer2) - 1] != '\n'))
					readed = recv(id, &buffer2[strlen(buffer2)], 1, 0);
				if (readed == 0) // <=
				{
					sprintf(buffer, "server: client %d just left\n", db[id]);
					send_all(buffer, server, id, size);
					FD_CLR(id, &new_fd);
					close(id);
				}
				else
				{
					sprintf(buffer, "client %d: %s", db[id], buffer2);
					send_all(buffer, server, id, size);
				}
			}
		}
	}
	return (0);
}