# Multi Client Message App C++

This is a simple multi client - server console chat application with minimal aesthetic features (see screenshot below). This runs purely in terminal, and if you pull this on unix/mac you won't need to install any libraries. If you are on Linux you will most likely need to install nucurses.

## :no_entry_sign:WINDOWS USERS ARE STRICTLY PROHIBITED FROM USING THIS APPLICATION:no_entry_sign:
Even if you can get this to run on Windows you still do not have my permission to use it. Run Fedora. Or buy a mac.

### Usage:
For client side: <br>
compile with 'g++ -std=c++11 ClientChat.cpp -o client -lncurses' <br>
run './client "username"'

For server side do the same but for the server file, no arguments.

### Sample:
Note the right terminal is the server and the left is local. Apologies for the subpar server side user experience, but the main focus was client side.

<img width="1680" alt="Screenshot 2024-08-01 at 2 06 53 PM" src="https://github.com/user-attachments/assets/9843bb19-3583-4604-a787-29f98a542704">

### Currently building a standalone widget application for this with many more features!
