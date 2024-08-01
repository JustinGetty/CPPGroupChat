#include <iostream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <functional>
#include <ncurses.h>
#include <ctime>


//TODO get col value, set max input length to that value - name/date

WINDOW* output_win;
WINDOW* input_win;

struct Message
{
	std::string sender;
	std::string message;
	std::string date;
	
};

void receive_messages(int client_socket)
{

    char buffer[1024];
	std::string accumulated_data;

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            std::cerr << "Error recoeving data or connection closed by server" << std::endl;
			break;
        }

		accumulated_data.append(buffer, bytes_received);

        size_t pos;
        while ((pos = accumulated_data.find('\n')) != std::string::npos)
        {
            std::string message = accumulated_data.substr(0, pos + 1);
            accumulated_data.erase(0, pos + 1);

            if (!message.empty() && message.back() == '\n') {
                message.pop_back();
            }

            wmove(output_win, getcury(output_win), 1);
            wprintw(output_win, "%s\n", message.c_str());
            wrefresh(output_win);
			
			wmove(input_win, 1, 1);
			wclrtoeol(input_win);
			wrefresh(input_win);
        }				

    }
}

void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    //curs_set(2);
    keypad(stdscr, TRUE);
}

void end_ncurses() {
    endwin();
}

Message set_message(Message* message, const std::string& mess, std:: string name) {
    std::time_t raw_time = std::time(nullptr);

    std::string date = std::ctime(&raw_time);

    if (!date.empty() && date.back() == '\n') {
        date.pop_back();
    }

    std::string day_date = date.substr(4, 6);
    if (day_date[4] == ' ')
    {
        day_date[4] = '0';
    }
    std::string day_time = "[" + day_date + " " + date.substr(11, 5) + "]";

    message->date = day_time;
    message->message = mess;
    message->sender = name;

    return *message;
}


int main(int argc, char *argv[])
{
    init_ncurses();

	std::string name = argv[1];

    //get terminal window size
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

	output_win = newwin(rows - 3, cols, 0, 0);
    input_win = newwin(3, cols, rows - 3, 0);

    //grab dimensions of current window, following lines structure window
    scrollok(output_win, TRUE);

    box(output_win, 0, 0); //box around input window
    box(input_win, 0,0);
    wrefresh(output_win); //refresh with new settings
    wrefresh(input_win);

    wmove(output_win, getcury(output_win), 1);
	wprintw(output_win, "Welcome to the chat!\n");
	wrefresh(output_win);
    box(input_win, 0, 0);


    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        std::cerr << "Error creating the socket ..." << std::endl;
    }
    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(4000);
    clientAddr.sin_addr.s_addr = inet_addr("192.168.65.134");


    if (connect(client_socket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == -1)
    {
            std::cerr << "connection failed ruh roh" << std::endl;
    }

    //std::cout << "connection successfull" << std::endl;

    //recieve thread (for simultaneous send/receive non-blocking io)
    std::thread rec_thread([client_socket]() { receive_messages(client_socket); });
    rec_thread.detach();


    char mess[1024];
    while (true)
    {
      
        Message message;

        wmove(input_win, 1, 1);
        wclrtoeol(input_win);
        box(input_win, 0, 0);
        wrefresh(input_win);
		
		echo();
        wgetnstr(input_win, mess, sizeof(mess) - 1);
		noecho();

        message = set_message(&message, mess, name);
        std::string complete_message = message.date + " " + message.sender +  ": " + message.message + "\n";

		wmove(output_win, getcury(output_win), 1);
        wprintw(output_win, complete_message.c_str()); 
        box(output_win, 0, 0);
        wrefresh(output_win);

        send(client_socket, complete_message.c_str(), strlen(complete_message.c_str()), 0);
        if (strcmp(mess, "exit\n") == 0)
            break;
        memset(mess, 0, sizeof(mess));
    }
    close(client_socket);

	return 0;
}

