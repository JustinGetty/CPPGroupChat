#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <functional>


void receive_messages(int client_socket)
{
    char buffer[1024];
	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
		if (bytes_received <= 0)
		{
		    std::cerr << "Error recoeving data or connection closed by server LOL it broke lmaooo" << std::endl;
		}
		
		std::cout << "New Message: " << buffer << std::endl;
	}
}


int main()
{

	std::cout << "Starting the Client ..." << std::endl;
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == -1)
	{
		std::cerr << "Error creating the socket ..." << std::endl;
	}
	
	sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(4000);
	clientAddr.sin_addr.s_addr = inet_addr("192.168.65.134");
	
	//probably add error handling here, jk prob not :)	
	connect(client_socket, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
	
	//recieve thread (for simultaneous send/receive non-blocking io)
	std::thread rec_thread([client_socket]() { receive_messages(client_socket); });
	rec_thread.detach();


	char mess[1024];
	while (true)
	{
		std::cout << "Enter the message to send: ";
		fgets(mess, sizeof(mess), stdin);
		send(client_socket, &mess, strlen(mess), 0);
		if (strcmp(mess, "exit\n") == 0)
			break;
		memset(mess, 0, sizeof(mess));
	}
	close(client_socket);
	return (0);
}


