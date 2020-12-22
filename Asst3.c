#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <time.h>
#include <ctype.h>
#include <netdb.h>



#define CT_ERR 1
#define LN_ERR 2
#define FT_ERR 3

typedef struct joke
{
    char* setup;
    char* punch;
}JK;


void error(char* );
void bind_server_socket(int ,int );
void connect_client(int ,JK** ,int);
void toClient(int ,char* );
int port_checker(char* );
int LN_checker(char* );
int CT_checker(char* ,int ,JK* );
int get_msg_len(char* ,int* );
int read_in_err(int ,char* );
int read_in(int ,int ,JK* );
int get_jk_list(JK** ,char* );
int create_server(char* );
char* get_errcode(int ,int );
char* talk(JK* ,int );
JK* get_jk(JK** ,int );
JK* new_joke(char* ,char* );



int main(int argc, char *argv[])
{
    int port,sfd,er,max_jk;
    struct addrinfo hint, *addr_list, *addr;

    if(argc < 2)
    {
        printf("Please enter a port!!!\n");
        exit(EXIT_FAILURE);
    }
    port_checker(argv[1]);

    sfd = create_server(argv[1]);
    if(sfd < 0)
    {
        exit(EXIT_FAILURE);
    }

    JK* list[100];
    if(argc == 3)
    {
        max_jk = get_jk_list(list,argv[2]);  
    }

    if(argc == 2)
    {
        printf("The default joke is going to be delivered....\n");
        char temp[100] = "Who ";
        char temp2[100] = "I didn't know you were an owl! "; 
        list[0] = new_joke(temp,temp2);
        max_jk = 0;
    }

    connect_client(sfd,list,max_jk);

    return EXIT_SUCCESS;
}


/*
 * Function: create_server
 * ----------------------------
 *  This function creats a server socket, and reports error if it occurrs
 *
 *  port : user input port
 * 
 *  return : the socket descripter
 */
int create_server(char *port)
{
    struct addrinfo hint, *addr_list, *addr;
    int error, sfd;


    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    error = getaddrinfo(NULL, port, &hint, &addr_list);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    for (addr = addr_list; addr != NULL; addr = addr->ai_next) {
        sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        
        if (sfd == -1) {
            continue;
        }

        
        if ((bind(sfd, addr->ai_addr, addr->ai_addrlen) == 0) &&
            (listen(sfd, 10) == 0)) {
            break;
        }

        close(sfd);
    }

    if (addr == NULL) {
        fprintf(stderr, "Failed to bind\n");
        return -1;
    }

    freeaddrinfo(addr_list);

    return sfd;
}


/*
 * Function: connect_client
 * ----------------------------
 *  This function will keep accepting clients and never end
 *
 *  sfd : server socket
 *  list: the joke list
 *  max_jk : the number of jokes list contains
 * 
 */
void connect_client(int sfd, JK** list, int max_jk)
{

    int cfd;
    for(;;)
    {
        puts("\nWaiting for connection.....");

        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(struct sockaddr_storage);

        cfd = accept(sfd, (struct sockaddr *)&client_addr, &addr_size);

        
        if (cfd == -1)
        {
            printf("Failed to accept!\n");
            continue;
        }
        toClient(cfd, "Internet Knock-Knock Protocol Server\r\nBy Hsinghui Ku,\tChang Li\r\n\n");

        puts("Server\t\t\t\tClient");
        puts("---------\t\t\t---------");
        JK* rand_jk = get_jk(list,max_jk);

        for(int i = 0; i < 6; i++)
        {
            if (i % 2 == 0)
            {
                char* temp = talk(rand_jk,i);
                toClient(cfd, temp);
                free(temp);
            }
            else
            {
                int error = read_in(cfd,i,rand_jk);

                if(error < 0)
                {
                    break;
                }
                if(error > 0)
                {
                    char* errcode = get_errcode(error,i);

                    toClient(cfd, errcode);
                    free(errcode);
                    break;
                }
            }
        }
        fflush(stdin);
        close(cfd);
    }

}

/*
 * Function: new_joke
 * ----------------------------
 *  This function constructs the space for a new joke,
 *  and assigne the setup line and punch line
 *
 *   str1 : setup line from reading text file
 *   str2 : punch line from reading text file
 *
 *   returns: JK* pointer to the new joke
 */
JK* new_joke(char* str1, char* str2)
{
    JK* new = (JK*)malloc(sizeof(JK));
    int len = strlen(str1);
    str1[len-1] = '\0';
    len = strlen(str2);
    str2[len-1] = '\0';  
    new->setup = strdup(str1);
    strcpy(new->setup,str1);
    new->punch = strdup(str2);
    strcpy(new->punch,str2);
    return new;
}

/*
 * Function: toclient
 * ----------------------------
 *  This function send message to client and print it out.
 *
 *  cfd : client socket 
 *  s : the string being sent
 */
void toClient(int cfd, char *s)
{
    int result = write(cfd, s, strlen(s));
    printf("%s\n",s);
    if (result == -1)
    {
        printf("failed talking to the client\n");
    }
}

/*
 * Function: port_checker
 * ----------------------------
 *  This function check if the port numer is valid
 *
 *  argv : 
 *  
 */
int port_checker(char* argv )
{
    int len = strlen(argv);
    int a = 0;
    int b = 0;

    for( int i=0; i<len; i++)
    {
        if(argv[0]==argv[i])
        {
            a++;    
        }
        if((argv[i]==argv[i+1]+1) || (argv[i]==argv[i+1]-1))
        {
            b++;
        }
    }
    int port = atoi(argv);
    if(port<=5000 || port>=65536)
    {
        printf("Please enter a port between 5000 and 65536\n\n");
        exit(EXIT_FAILURE);
    }
    if( a==len || b==len-1 )
    {
        printf("Please enter a port that does not involve repetition or simple patterns like 11111 or 12345.\n\n");
        exit(EXIT_FAILURE);
    }
    return 1;
}

/*
 * Function: LN_checker
 * ----------------------------
 *  This function checks if the length of message is correct
 *  and it also checks if the length values are valid digits 
 *
 *  s : the string being checked
 *
 *   returns: 
 *          FT_ERR if the length field is not digit or blank
 *          LN_ERR if the length of message is not matching with the length
 *          0 if all are correct
 */
int LN_checker(char *s)
{
    char *b;
    const char a[2] = "|";
    int Message_length,len;
    char message[100];
    strcpy(message,s);

    /*message type*/
    strtok(message, a);

    /*Length string*/
    b = strtok(NULL,a);
    if(b == NULL)
    {
        return FT_ERR;
    }
    Message_length = atoi(b);

    /*content*/
    b = strtok(NULL,a);
    if(b == NULL)
    {
        return LN_ERR;
    }
    len = strlen(b);
    if(Message_length == 0)
    {
        return FT_ERR;
    }
    if(Message_length != len)
    {
        return LN_ERR;
    }
    return 0;
}

/*
 * Function: CT_checker
 * ----------------------------
 *  This function checks if the content of the message is correct.
 *
 *  s : the string being checked
 *  msg_num : indicating which message is going to be checked with.
 *            (receeiving from client, it should be odd number from 1 - 5)
 *  jk : the random joke that is generated by server
 *
 *   returns: 
 *          CT_ERR if the content is not correct
 *          0 if all are correct
 */
int CT_checker(char *s, int msg_num,JK* jk)
{
    char *b;
    int len;
    int ret = 0;
    
    char message[1000]; 
    char temp[1000];
    strcpy(temp,"");   
    strcpy(message,s);
    strtok(message, "|");
    strtok(NULL,"|");
    b = strtok(NULL,"|");

    switch (msg_num)
    {
        case 1:
            if(strcmp("Who's there?",b) != 0)
            {
                printf("CT:%s\n",b);
                ret = CT_ERR;
            }
            break;

        case 3:
            sprintf(temp,"%s, who?",jk->setup);
            if(strcmp(temp,b) != 0)
            {
                ret = CT_ERR;
            }
            break;

        case 5:
            len = strlen(b);
            if( b[len-1]!='.' && b[len-1]!='!' && b[len-1]!='?' )
            {
                ret = CT_ERR;
            }
            break;        
        default:
            ret = 0;
    }
    return ret;
}

/*
 * Function: get_msg_len
 * ----------------------------
 *  This function try to get the length field number of message, 
 *  and indicate error.
 *
 *  str : the string being checked
 *  count_ptr : point to "count" which counts the characters have been read
 *              after the second bar.  
 *
 *  return: 
 *          non-neg number if the message length is valid.
 *          -1 if the length is invalid or blank  
 */
int get_msg_len(char* str,int* count_ptr)
{
    char *b;
    int ret = 0;
    const char a[2] = "|";
    char message[100];
    char temp[100];
    strcpy(message,str);

    /*message type*/
    strtok(message, a);

    /*Length string*/
    b = strtok(NULL,a);
    if(b == NULL)
    {
        return -1;
    }
    int len = strlen(b);

    /*check if the chars between two bars are all digits*/
    for(int i = 0; i<len ;i++)
    {
        if(!(isdigit(b[i])))
            return -1;
        if(b[i]=='|')
            break;
    }

    /*convert string to int*/
    ret = atoi(b);

    b = strtok(NULL,a);

    /*noting after second bar in current msg*/
    if(b == NULL)
        return ret;

    /* count the character after second bar*/
    int i = 0;
    len = strlen(b);
    while(b != NULL && i < len)
    {
        if(b[i++]!='|')
        (*count_ptr)++;
    }

    return ret;
}

/*
 * Function: read_in_err
 * ----------------------------
 *  This function reads the rest of error message if a message begins with "ERR|"
 *
 *  cfd : client socket
 *
 *  returns: -1 indicating the end of Error msg
 */
int read_in_err(int cfd,char* buf)
{
    char buffer[200];
    strcpy(buffer,buf);

    int err = 0;
    int len = strlen(buf);
    int count = len - 4;

    while(count < 5)
    {
        char temp[200];
        err = read(cfd,temp,sizeof(temp));
        
        if(err < 0)
        {
            continue;
        }
        temp[err-1]='\0';
        len = strlen(temp);
        count += len-1;
        strncat(buffer,temp,len-1);

    }
    printf("\t\t\t\t%s\n",buffer);
    return -1;
}

/*
 * Function: read_in
 * ----------------------------
 *  This function keeps reading the bytes sent from client, 
 *  until the following three conditions:
 *      1. Format is obivious invalid
 *      2. Read at most the number of bytes in the length filed
 *      2. encounter the third pipe
 *
 *  cfd : client socket
 *  msg_num :  indicating which message is going to be checked with.
 *            (receeiving from client, it should be odd number from 1 - 5)
 *  jk : the random joke that is generated by server
 * 
 *  returns: 
 *          FT_ERR, CT_ERR, LN_ERR if the msg is incorrect
 *          -1 if server receives the ERR msg
 *          0 if the msg is entirely correct
 */
int read_in(int cfd,int msg_num,JK* jk)
{
    char buffer[400];
    strcpy(buffer,"");
    int read_in = 0;
    int bar = 0;
    int max_bar = 0;
    int ret = 0; 
    int msg_len = 0;
    int count = 0;
    int err,len;
    
    do
    {
        char temp[200];
        
        err = read(cfd,temp,sizeof(temp));

        /* Getting nothing from client */
        if(err < 0)
        {
            continue;
        }

        /* Remove new line in the end of client sent msg*/
        temp[err-1]='\0';
        len = strlen(temp);

        /* count the pipe(bar)*/
        int i = 0;
        for(;i<len-1;i++)
        {
            if(temp[i]=='|')
                bar++;
            
            if( bar >= 3 )
            {
                temp[i+1] = '\0';
                i++;
                break;
            }                     
        }
        /* Count how many characters have been received */
        /* and concatenate to the buffer.                */
        read_in += i;
        strncat(buffer,temp,i);

        /* count the character after second bar*/   
        if(msg_len>0)
        {
            count += i;
        }

        /*determine the message type*/
        if( max_bar == 0 && read_in >= 4)
        {
            if(strncmp(buffer,"REG|",4)==0)
            {
                max_bar = 3;    
            }
            else if(strncmp(buffer,"ERR|",4)==0)
            {
                max_bar = 2;
                return read_in_err(cfd,buffer);
            }
            else
            {
                ret = FT_ERR;
                return ret;
            }  
        }
        
        /* if got the second pipe, then try to get msg length field */
        if(max_bar == 3 && bar == 2 && msg_len == 0)
        {
            msg_len = get_msg_len(buffer,&count);
            if(msg_len < 0)
                return FT_ERR;
        }
        
        /* Read at most the length of length field number+1*/
        if(msg_len != 0 && count > msg_len)
            break;

    }while(max_bar == 0 || bar < max_bar);
    

    /*The last character should be '|'*/    
    len = strlen(buffer);
    if(buffer[len-1] != '|')
        return FT_ERR;

    /* Check the msg */
    ret = LN_checker(buffer);
    if(ret > 0)
    {
        return ret;
    }
    else
        ret = CT_checker(buffer,msg_num,jk);

    /* Everyting is correct, then print it out*/
    if(ret == 0)
    {
        printf("\t\t\t\t%s\n",buffer);
    }

    return ret;
}

/*
 * Function: get_errcode
 * ----------------------------
 *  This function converts the int ERR number into a formatted ERR message
 *
 *  er : the int error number indicating CT, LN, or FT
 *  msg_ num : the number of current msg
 * 
 *  returns: the converted ERR msg
 */
char* get_errcode(int er,int msg_num)
{
    char* ret = (char*)malloc(12);
    switch (er)
    {
        case CT_ERR:
            sprintf(ret,"ERR|M%dCT|",msg_num);
            break;
        case LN_ERR:
            sprintf(ret,"ERR|M%dLN|",msg_num);
            break;
        case FT_ERR:
            sprintf(ret,"ERR|M%dFT|",msg_num);
            break;
    }
    return ret;
}

/*
 * Function: get_jk_list
 * ----------------------------
 *  This function reads jokes from the given path into the joke list, 
 *  and count how many jokes in total
 *  
 *  jk : the joke list
 *  path : the path user entered
 * 
 *  returns: the number of jokes in total
 */
int get_jk_list(JK** list,char* path)
{

    FILE *file = fopen(path, "r");
    int i = 0;
    if(file == NULL)
    {      
        printf("Cannot access joke file\n");
        return 0;
    }

    void* end;

    do
    {
        char temp[1000];
        char temp2[1000];
        end = fgets(temp,999,file);
        end = fgets(temp2,999,file);
        list[i] = new_joke(temp,temp2);
        end = fgets(temp,999,file);
        i++;
    }while(end != NULL);

    return i;
}

/*
 * Function: get_jk
 * ----------------------------
 *  This function generates a random joke from the list
 *  
 *  list : the joke list
 *  max : the number of joke we have
 * 
 *  returns: the JK* points to the address of random joke
 */
JK* get_jk(JK** list,int max)
{
    srand(time(NULL));
    int ran = rand() % (max + 1);

    return list[ran];
}

/*
 * Function: talk
 * ----------------------------
 *  This function generates the formatted regular message
 *  
 *  jk : the random joke
 *  msg_num : the number of current message
 * 
 *  return: a string of formatted message 
 */
char* talk(JK* jk,int msg_num)
{
    char* temp = (char*)malloc(1000);
    int len;

    switch (msg_num)
    {
        case 0: 
            sprintf(temp,"REG|13|%s|","Knock, knock.");
            break;
        case 2:
            len = strlen(jk->setup);
            sprintf(temp,"REG|%d|%s.|",len+1,jk->setup);
            break;
        case 4:
            len = strlen(jk->punch);
            sprintf(temp,"REG|%d|%s|",len,jk->punch);
            break;
    }
    return temp;
}

