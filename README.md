# Socket Programming

## What is a socket?
A socket is a mechanism for connecting two processes on one or more machines. In this connection, the socket is like an end to which we send or receive information.
The socket is actually an abstraction for the lower layers of the operating system that can make this connection.


## How to run the program?
In this project, we have a central server that can create rooms for asking questions for a number of fields of study (computer-electricity-civil-mechanics). This server always listens on a specific port to connect to clients. People can connect to the server as a client and give the server the number of strings they want to enter and ask questions.

Note that the server is a process and each client is a separate process. The capacity of each room is 3 people.

As soon as the number of people in a room reaches the quorum, the server announces a new port to the people in that room so that they can bred messages on that port.

Each client communicates with the server by TCP, and after a room is filled, the clients communicate with each other by UDP and broadcast. Clients can ask questions in turn, and this question will be broadcast to everyone in the room, and other users can take turns answering this question.

In the end, after everyone has answered the question, the person who asked the question announces the best answer to all the people in the room and then sends a message containing his question and the answers of the others to the server. The server has a file to collect all the questions and answers and adds the string of messages received by the person who asked the question at the end of this file.

After everyone has asked their question and a round of questions is over, the room is finished and the client program is over.

### Client and server should run as follows:(ports must be personalized)

./server <server_port>

./client <server_port>
