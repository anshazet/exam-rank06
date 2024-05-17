#include <stdio.h>		// Standard input/output definitions
#include <stdlib.h>		// Standard library for malloc, free, exit, etc.
#include <unistd.h>		// Provides access to the POSIX operating system API
#include <string.h>		// String handling functions
#include <strings.h>	// More string handling functions
#include <netdb.h>		// Definitions for network database operations
#include <errno.h>		// Defines macros for reporting and retrieving error conditions
#include <sys/socket.h> // Main headers for socket-based networking
#include <sys/select.h> // Definitions for the select function, which allows to monitor multiple file descriptors
#include <netinet/in.h> // Constants and structures needed for internet domain addresses

// Function to handle fatal errors
void ft_error()
{
	write(2, "Fatal error\n", 12); // Write the error message to stderr
	exit(1);					   // Exit the program with a status of 1
}

// Function to send a message to all clients except the server and the sender
void send_all(char *buffer, int server_fd, int client_fd, int size_fd)
{
	for (int i = 0; i <= size_fd; i++)
		if (i != server_fd && i != client_fd)
			send(i, buffer, strlen(buffer), 0); // Send buffer to all clients except the server and the one who sent the message
}

// Main function
int main(int ac, char **av)
{
	int server_fd, client_fd, size_fd, readed;
	int db[65535] = {0}, limit = 0;		  // Database of client IDs, limited arbitrarily to 65535 for demonstration
	struct sockaddr_in server_addr;		  // Structure to hold server address
	fd_set old_fd, new_fd;				  // File descriptor sets for select
	char buffer[200000], buffer2[150000]; // Buffers for storing messages

	// Check if the program was started with exactly one argument
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	// Create a socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
		ft_error();

	// Set the server address
	bzero(&server_addr, sizeof(server_addr));		 // Zero out the structure
	server_addr.sin_family = AF_INET;				 // Internet address family
	server_addr.sin_addr.s_addr = htonl(2130706433); // Host TO Network Long Address to bind to (127.0.0.1)
	server_addr.sin_port = htons(atoi(av[1]));		 // Host TO Network Short Set port number from argument

	// Bind the socket to the port
	if ((bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr))) != 0)
		ft_error();

	// Listen on the socket
	if (listen(server_fd, 10) != 0)
		ft_error();

	// Initialize file descriptor sets
	FD_ZERO(&old_fd);
	FD_ZERO(&new_fd);
	FD_SET(server_fd, &new_fd); // Add the server socket to the new file descriptor set
	size_fd = server_fd;		// Initialize the maximum file descriptor number

	// Enter an infinite loop to handle incoming connections and data
	while (1)
	{
		old_fd = new_fd; // Copy the new file descriptor set to old_fd for use in select
		// Wait for an activity on any of the sockets
		if (select(size_fd + 1, &old_fd, NULL, NULL, NULL) < 0)
			ft_error();

		// Loop through all possible socket numbers to check for activity
		for (int id = 0; id <= size_fd; id++)
		{
			if (FD_ISSET(id, &old_fd) == 0) // Check if the current fd is part of the set
				continue;
			if (id == server_fd) // New connection request on the original socket
			{
				// Accept a new connection
				if ((client_fd = accept(server_fd, NULL, NULL)) < 0)
					ft_error();
				if (client_fd > size_fd)
					size_fd = client_fd;	// Update the maximum file descriptor number
				db[client_fd] = limit++;	// Assign a unique client ID
				FD_SET(client_fd, &new_fd); // Add to the set of active file descriptors
				// Notify all clients of a new connection
				sprintf(buffer, "server: client %d just arrived\n", db[client_fd]);
				send_all(buffer, server_fd, client_fd, size_fd);
			}
			else // Data arriving on an already-connected socket
			{
				bzero(buffer, 200000); // Clear the buffer
				bzero(buffer2, 150000);
				readed = 1;
				// Receive data until newline
				while (readed == 1 && (!buffer2[0] || buffer2[strlen(buffer2) - 1] != '\n'))
					readed = recv(id, &buffer2[strlen(buffer2)], 1, 0);
				if (readed <= 0) // If no data, the client disconnected
				{
					sprintf(buffer, "server: client %d just left\n", db[id]);
					send_all(buffer, server_fd, id, size_fd);
					FD_CLR(id, &new_fd); // Remove from the set of active file descriptors
					close(id);			 // Close the client socket
				}
				else // Otherwise, send received data to all clients
				{
					sprintf(buffer, "client %d: %s", db[id], buffer2);
					send_all(buffer, server_fd, id, size_fd);
				}
			}
		}
	}
	return 0; // Not reachable
}
