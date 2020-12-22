# Knock-Knock-Joke-Server
This is Assignment3 of CS 214:System Programming, Fall 2020\
Completed by Hsinghui Ku & Chang Li
## Description
This program use C socket to implement a simple network service: a knock knock joke server. It will use the protocal desplayed below. There is an application protocol and a message format. The application protocol specifies what messages of what type are allowed when. The message format specifies how the application messages should be constructed and read/written from/to the socket.

> Knock, knock.\(Server\)\
> Who's there?\(Client\)\
> \<setup line\>.\(Server\)\
> \<setup line\>, who?\(Client\)\
> \<punch line\>\<punctuation\>\(Server\)\
> \<expression of annoyance and/or disgust or surprise\>\<punctuation\>\(Client\)

**This program will take the first argument that is the port number it should start on, and the second argument to this program a file that holds a joke list. The joke list is
newline-separated setup and punch lines.**

### Error Handling
If ever a message is sent that does not match the requirements of the protocol, the side receiving the
bad message should not send the next message but instead respond with an error message (see the KKJ messaging format below) indicating the error type and shut down their context.

### Message Format
The first three characters are the message type. REG denotes a regular message that should be read in, checked and if it holds to the requirements, responded to normally. ERR denotes an error message that should be read in and handled, however the protocol is now ended and the side receiving the error can deallocate and shut down. Once an error message has been sent, the sender should deallocate and shut down.

#### Regular Message
A regular message is a message that should be read in, checked and if it holds to the requirements,
responded to according to the KKJ application protocol.\
Every message delivered that is not an error message should have the following format:\
REG|\<length segment\>|\<message segment\>|\
The message length includes only the length of the message segment and not the '|' separators. \
e.g.:\
To send the “Knock, knock.” message: REG|13|Knock, knock.|\
To send the “Who's there?” message: REG|12|Who's there?|

#### Error Messages:
An error message is a message that should be read in, checked and handled, however no response
is sent. An error message halts the protocol and causes both the sender and receiver to deallocate and shut down after sending/receiving.\
The default handling action is to output a textual description of the the error code on standard out.\
Every message denoting an error should have the following format:
ERR|\<error code\>|\
ERR: the characters 'E', 'R', 'R' in sequence. \
|: a single '|' character\
\<error code\>: one of the error codes below. \
|: a single '|' character

```
M0CT - message 0 content was not correct (i.e. should be “Knock, knock.”)
M0LN - message 0 length value was incorrect (i.e. should be 13 characters long)
M0FT - message 0 format was broken (did not include a message type, missing or too many '|')
M1CT - message 1 content was not correct (i.e. should be “Who's there?”)
M1LN - message 1 length value was incorrect (i.e. should be 12 characters long)
M1FT - message 1 format was broken (did not include a message type, missing or too many '|')
M2CT - message 3 content was not correct (i.e. missing punctuation)
M2LN - message 2 length value was incorrect (i.e. should be the length of the message)
M2FT - message 2 format was broken (did not include a message type, missing or too many '|')
M3CT - message 3 content was not correct
(i.e. should contain message 2 with “, who?” tacked on)
M3LN - message 3 length value was incorrect (i.e. should be M2 length plus six)
M3FT - message 3 format was broken (did not include a message type, missing or too many '|')
M4CT - message 4 content was not correct (i.e. missing punctuation)
M4LN - message 4 length value was incorrect (i.e. should be the length of the message)
M4FT - message 4 format was broken (did not include a message type, missing or too many '|')
M5CT - message 5 content was not correct (i.e. missing punctuation)
M5LN - message 5 length value was incorrect (i.e. should be the length of the message)
M5FT - message 5 format was broken (did not include a message type, missing or too many '|')
```
