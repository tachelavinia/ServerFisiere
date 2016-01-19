/*
*      PROTOCOALE DE COMUNICATIE - TEMA 1
* 				Simularea unui server de fisiere
*                    realizata de Lavinia Tache 
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001


int getBit(int position, char chr){
	return (( chr >> position ) & 1);
}

char setBit(int position, char chr){
	chr |= 1 << position;
	return chr;
}

/* Functia Hamming de codare a unui mesaj*/
char * code(char * message, int len){
	char * messageCoded = (char *)calloc(1400, sizeof(char));
	int i,k;
	int counter = 0;
	for(k = 0; k < len ; k++){
		int bits[13], bitsForTwoChar[17];
		memset(bits, 0, sizeof(bits));
		memset(bitsForTwoChar, 0, sizeof(bitsForTwoChar));
		
		int contor = 7;
		for(i = 1; i < 13; i++){
			if(i != 1 && i != 2 && i != 4 && i != 8){
				bits[i] = getBit(contor, message[k]);
				contor--;
			}
		}
		int P1 = (bits[3] + bits[5] + bits[7] + bits[9] + bits[11]) % 2;
		int P2 = (bits[3] + bits[6] + bits[7] + bits[10] + bits[11]) % 2;
		int P4 = (bits[5] + bits[6] + bits[7] + bits[12]) % 2;
		int P8 = (bits[9] + bits[10] + bits[11] + bits[12]) % 2;
	
		bits[1] = P1;
		bits[2] = P2;
		bits[4] = P4;
		bits[8] = P8;
		
		for(i = 5; i < 17; i++){
			bitsForTwoChar[i] = bits[i-4];
		}
		char first = 0;
		char second = 0;
		for(i = 1; i <= 8; i++){
			if(bitsForTwoChar[i] == 1){
				first |= (1 << (8-i));
			}
		}
		for(i = 9; i <= 16; i++){
			if(bitsForTwoChar[i] == 1){
				second |= (1 << (16 - i));
			}
		}
	
	messageCoded[counter] = first;
	messageCoded[counter + 1] = second;
	counter+=2;
	}
	return messageCoded;
} 

/* Functia Hamming de decodare a unui caracter*/
char decode(char chr[2]){
	int bits[13],i;
	for(i = 1; i < 13; i++){
		bits[i] = 0;
	}
	int contor = 1;
	for(i = 3; i >= 0; i--){
		bits[contor] = getBit(i,chr[0]);
		contor++;
	}
	for(i = 7; i >= 0; i--){
		bits[contor] = getBit(i, chr[1]);
		contor++;
	}
	int P1 = ( bits[3] + bits[5] + bits[7] + bits[9] + bits[11]) % 2;
	int P2 = ( bits[3] + bits[6] + bits[7] + bits[10] + bits[11]) % 2;
	int P4 = ( bits[5] + bits[6] + bits[7] + bits[12]) % 2;
	int P8 = ( bits[9] + bits[10] + bits[11] + bits[12]) % 2;
	int wrongParityBit = 0;
	
	if(P1 != bits[1]){
		wrongParityBit += 1;
	}
	if(P2 != bits[2]){
		wrongParityBit += 2;
	}
	if(P4 != bits[4]){
		wrongParityBit += 4;
	}
	if(P8 != bits[8]){
		wrongParityBit += 8;
	}
	if(wrongParityBit != 0){
		if(bits[wrongParityBit] == 1){
			bits[wrongParityBit] = 0;
		}else{
			bits[wrongParityBit] = 1;
		}	
	}
	
	char messageToReturn = 0;
	char position = 0;
	for(i = 1; i < 13; i++){
		if(i != 1 && i != 2 && i != 4 && i != 8){
			position ++;
			if(bits[i] == 1){
				messageToReturn |= (1 << ( 7 - (position - 1)));
			}
		}
	}
	return messageToReturn;	
}

char * correctMessage(char *message, int len){
	char buffer[3];
	int i = 0;
	char * messageCorrected = (char *) malloc(700 * sizeof(char));
	char correct;
	int contor = 0;
	for(i = 0; i < len; i += 2){
		buffer[0] = message[i];
		buffer[1] = message[i+1];
		buffer[2] = '\n';
		correct = decode(buffer);
		messageCorrected[contor] = correct;
		contor++;
	}
	return messageCorrected;
}

int numberOfDigits(int n){
	int count = 0;
	while( n != 0){
		n /= 10;
		++count;
	}
	return count;
}

int getMessageParity( char * message, int len){
	int i, messageParity = 0, j;
	for(i = 0; i < len; i++){
		for(j = 0; j < 8; j++){
			messageParity += ((message[i] >> j) & 1);
		}
	}
	return messageParity % 2;
}

int getNumberParity(int dim){
	int i, numberParity = 0;
	for(i = 0; i < 16; i++){
		numberParity += ((dim >> i) & 1);
	}
	return numberParity % 2;
}

int checkParity(char *message, int len, char parityByte){
	if( getMessageParity(message, len) == (parityByte & 1))
		return 1;
	return 0;
}

int sendACK(){
	msg s;
	sprintf(s.payload, "%s", "ACK");
	s.len = 4;
	int check = send_message(&s);
	return check;
}

int sendNACK(){
	msg s;
	sprintf(s.payload, "%s", "NACK");
	s.len = 5;
	int check = send_message(&s);
	return check;
}

int sendDimension(int length){
	msg s;
	sprintf(s.payload, "%d", length);
	s.len = strlen(s.payload) + 1;
	int check = send_message(&s);
	return check;
}

int sendDimensionWithParity(int length){
	msg s;
	sprintf(s.payload, "%d%d", getNumberParity(length),length);
	s.len = strlen(s.payload) + 1;
	int check = send_message(&s);
	return check;
}

int sendMessageWithParity(char * message, int len){
	msg s;
	sprintf(s.payload, "%d%s", getMessageParity(message, len), message);
	s.len = strlen(s.payload + 1) + 1;
	int check = send_message(&s);
	return check;
}
int sendMessage(char * message){
	msg s;
	sprintf(s.payload, "%s", message);
	s.len = strlen(s.payload) + 1;
	int check = send_message(&s);
	return check;
}

int main(int argc, char ** argv)
{
	msg r;
	int i, res;
	char command[255], file[255];
	struct dirent *directory;
	DIR *pDir;
	
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
	
	for (i = 0; i < COUNT; i++) {
	/* Daca nu am argumente in linia de comanda se cere implementarea unui protocol simplu STOP AND WAIT. Pentru fiecare mesaj transmis se asteapta ACK , iar pentru fiecare mesaj primit se transmite ACK*/
		if(argc == 1){
			res = recv_message(&r);
			if (res < 0) {
				perror("[RECEIVER] Receive error. Exiting.\n");
				return -1;
			}
		/* Despart mesjul primit in comanda si fisierul/ directorul pe care se va aplica comanda si trimit ACK pentru comanda primita*/
			sscanf(r.payload, "%s%s", command, file);
			sendACK();
		/* Daca comanda este LS ii voi trimite clientului dimensiunea directorul , iar dupa ce voi primi ACK de la el ii voi trimite pe rand fisierele din director*/			
			if( strcmp (command, "ls") == 0){
				int len = 0;
				pDir = opendir(file);
				while((directory = readdir(pDir)) != NULL){
					len++;
				}
				closedir(pDir);
				res = sendDimension(len);
				if (res < 0) {
					perror("[RECEIVER] Send ACK error. Exiting.\n");
					return -1;
				}
				res = recv_message(&r);
				if( res < 0){
					perror("[RECEIVER] Receive error. Exiting.\n");
					return -1;
				}
				
				pDir = opendir(file);
				while((directory = readdir(pDir)) != NULL){
					res = sendMessage(directory->d_name);
					if (res < 0) {
						perror("[RECEIVER] Send ACK error. Exiting.\n");
						return -1;
					}
					res = recv_message(&r);
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}					
				}
				closedir(pDir);
			}
		/* Daca comanda primita este CD voi schimba directorul curent la directorul specificat in comanda -> file */
			if(strcmp (command, "cd") == 0){
				chdir(file);
			}
		/* Daca comanda este CP voi deschide fisierul primit si voi trimite clientului dimensiunea fisierului. Imediat dupa ACK trimit fragmente de lungimea dimensiunii maxime a unui mesaj ->MSGSIZE*/	
			if(strcmp (command, "cp") == 0){
				FILE *fp = fopen(file, "rb");
				
				fseek(fp, 0L, SEEK_END);
				int size = ftell(fp);
				fseek(fp,0L, SEEK_SET);
				
				res = sendDimension(size);
				if (res < 0) {
						perror("[RECEIVER] Send ACK error. Exiting.\n");
						return -1;
				}
				
				res = recv_message(&r);
				if( res < 0){
					perror("[RECEIVER] Receive error. Exiting.\n");
					return -1;
				}

				while(!feof(fp)){
					msg s;
					int len = fread(s.payload ,1,1400,fp);
					s.len = len;
					send_message(&s);
					res = recv_message(&r);
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}
				}
				fclose(fp);
			}
		/* Daca comanda este SN primesc de la client primesc de la client dimensiunea fisierului , iar in comanda numele acestuia. Voi creea un fisier cu numele new_file, unde file = nume fisier din comanda si ii voi trimite ACK. In continuare voi primi de la client fragmente din fisier pe care le voi scrie in fisierul specificat.*/	
			if(strcmp (command, "sn") == 0){
				recv_message(&r);
				long fileDimensions = atol(r.payload);
				res = sendACK();
				if( res < 0){
					perror("[RECEIVER] Receive error. Exiting.\n");
					return -1;
				}
				char * snFile = (char *) malloc(255 * sizeof(char));
				strcpy(snFile, "new_");
				strcat(snFile, file);
				FILE *fp = fopen(snFile, "wb");
				
				while(fileDimensions > 0){
					res = recv_message(&r);
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}
					int length = fwrite(r.payload, 1 , 1400, fp);
					res = sendACK();
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}
					fileDimensions -= length;
				}
				fclose(fp);
			}
			
		/* In caz de EXIT break */
			if(strcmp (command, "exit") == 0){
				break;
			}
		}
		else 
		
	/* In cazul in care argumentul din linia de comanda este PARITY atunci protocolul implementat va urmari detectia erorilor. Astfel, se calculeaza paritatea fiecarui mesaj. Aceasta este data de numarul de biti de 1 mod 2 rezultand o valoare de 1 in cazul in care numarul este impar, 0 in caz contrar. Valoarea calcultata este adaugata pe primul bit din primul byte al mesajului.
	  In momentul in care se primeste un mesaj se calculeaza paritatea mesajului si se calculeaza paritatea si se verifica daca este egala cu valoarea adaugata in primul byte. In cazul in care conditia nu se respecta se trimite NACK si se asteapta repetarea mesajului.
	  
	  Functiile implementate : LS, CD, CP, SN respecta structura descrisa anterior cu diferenta verificarii bitului de paritate*/
	  
		if(strcmp(argv[1], "parity") == 0){
			recv_message(&r);
			while( checkParity(r.payload + 1, r.len - 1, r.payload[0]) == 0){
				sendNACK();
				recv_message(&r);
			}
			res = sendACK();
			if( res < 0){
				perror("[RECEIVER] Receive error. Exiting.\n");
				return -1;
			}
			
			sscanf(r.payload + 1, "%s%s", command, file);
			
			if(strcmp(command, "ls") == 0){
				int len = 0;
				pDir = opendir(file);
				while((directory = readdir(pDir)) != NULL){
					len++;
				}
				closedir(pDir);
				res = sendDimensionWithParity(len);
				if (res < 0) {
					perror("[RECEIVER] Send ACK error. Exiting.\n");
					return -1;
				}
				res = recv_message(&r);
				if( res < 0){
					perror("[RECEIVER] Receive error. Exiting.\n");
					return -1;
				}
				while(r.len == 5){
					sendDimensionWithParity(len);
					recv_message(&r);
				}
				
				pDir = opendir(file);
				if(pDir == NULL){
					printf("Cannot open dorectory \n");
					return -1;
				}
				
				while((directory = readdir(pDir)) != NULL){
					sendMessageWithParity(directory->d_name, strlen(directory->d_name));
					recv_message(&r);
					while(r.len == 5){
						sendMessageWithParity(directory->d_name, strlen(directory->d_name));
						recv_message(&r);
					}
				}
				closedir(pDir);
			}
			
			if(strcmp(command,"cp") == 0){
				FILE *fp = fopen(file, "rb");
				fseek(fp, 0L, SEEK_END);
				int size = ftell(fp);
				fseek(fp, 0L, SEEK_SET);
				res = sendDimensionWithParity(size);
				if (res < 0) {
					perror("[RECEIVER] Send ACK error. Exiting.\n");
					return -1;
				}
				res = recv_message(&r);
				if( res < 0){
					perror("[RECEIVER] Receive error. Exiting.\n");
					return -1;
				}
				while(r.len == 5){
					sendDimensionWithParity(size);
					recv_message(&r);
				}
				int toSend = size;
				while(size > 0){
					char buffer[1399];
					int i = 0;
					msg s;
					int len = fread(buffer, 1, 1399, fp);
					s.payload[0] = getMessageParity(buffer, len);
					for(i = 0; i < len; i++){
						s.payload[i + 1] = buffer[i];
					}
					toSend -=len;
					s.len = len + 1; 
					res = send_message(&s);
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}
					res = recv_message(&r);
					if (res < 0) {
						perror("[RECEIVER] Send ACK error. Exiting.\n");
						return -1;
					}
					
					while(r.len == 5){
						
						res = send_message(&s);
						if( res < 0){
							perror("[RECEIVER] Receive error. Exiting.\n");
							return -1;
						}
						res = recv_message(&r);
						if (res < 0) {
							perror("[RECEIVER] Send ACK error. Exiting.\n");
							return -1;
						}		
					} 
					size -= len;
				}
				fclose(fp);
			}
			
			if(strcmp (command, "sn") == 0){
				recv_message(&r);
				while(!checkParity(r.payload + 1, r.len - 1, r.payload[0])){
					sendNACK();
					recv_message(&r);
				}
				res = sendACK();
				if( res < 0){
					perror("[RECEIVER] Receive error. Exiting.\n");
					return -1;
				}
				int dim = atoi(r.payload + 1);
				char * snFile = (char*)malloc(255 * sizeof(char));
				strcpy(snFile, "new_");
				strcat(snFile, file);
				FILE * fp = fopen(snFile, "wb");
				
				while( dim > 0){
					recv_message(&r);
					while(!checkParity(r.payload + 1, r.len, r.payload[0])){
						sendNACK();
						recv_message(&r);
					}
					res = sendACK();
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}
					int length = fwrite(r.payload + 1, 1, r.len - 1, fp);
					dim -= length;
				}
				fclose(fp);
			}
			
			if(strcmp (command, "cd") == 0){
				chdir(file);
			}
			
			if(strcmp (command, "exit") == 0){
				break;
			}
		}
	/* Pentru cazul in care parametrul din linia de comanda este HAMMING se va implementa un protocol care va detecta si corectarea eroare de un bit intr-un mesaj. Astfel, fiecare mesaj este codificat inainte de a fi transmis conform standardului. 
	Fiecare caracter ( 1 byte  = 8 biti ) va fi transformat in 16 biti. 8 dintre aceastia reprezinta bitii din caracterul codificat, 4 reprezinta bitii de control -> bitii indexati in puterile lui 2 , iar primii 4 biti sunt completati cu padding de 0.
	 La decodificare se corecteaza mesajul, detectarea bitului gresit fiind data de bitii de control.*/
		else
		 if(strcmp(argv[1], "hamming") == 0){
		 	msg s;
		 	res = recv_message(&r);
		 	if (res < 0) {
				perror("[RECEIVER] Send ACK error. Exiting.\n");
				return -1;
			}
			
			sscanf(correctMessage(r.payload, r.len) , "%s%s",	command, file);

			res = sendACK();
			if (res < 0) {
				perror("[RECEIVER] Send error. Exiting.\n");
				return -1;
			}
			
			if(strcmp(command, "ls") == 0){
				int len = 0;
				pDir = opendir(file);
				while((directory = readdir(pDir)) != NULL){
					len++;
				}
				closedir(pDir);

				char temp[sizeof(len)];
				temp[0] = len & 0xFF;
				temp[1] = (len >> 8) & 0xFF;
				temp[2] = (len >> 16) & 0xFF;
				temp[3] = (len >> 24) & 0xFF;
				
				char aux[32];
				sprintf(aux, "%d", len);
				
				char *buffer = code(aux, strlen(aux)+1);
				memcpy(s.payload, buffer, strlen(aux)*2);
				s.len = strlen(aux)*2;
				res = send_message(&s);
				
				if (res < 0) {
					perror("[RECEIVER] Send ACK error. Exiting.\n");
					return -1;
				}
				res = recv_message(&r);
				if( res < 0){
					perror("[RECEIVER] Receive error. Exiting.\n");
					return -1;
				}
				pDir = opendir(file);
				while((directory = readdir(pDir))){
					char* files = code(directory->d_name, strlen(directory->d_name));
					sprintf(s.payload, "%s", files);
					s.len = strlen(directory->d_name)*2;
					
					res = send_message(&s);
					if (res < 0) {
						perror("[RECEIVER] Send ACK error. Exiting.\n");
						return -1;
					}
					res = recv_message(&r);
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}					
				}
				closedir(pDir);
			}
			
			if(strcmp (command, "cd") == 0){
				chdir(file);
			}
			
			if(strcmp (command, "cp") == 0){
				FILE *fp = fopen(file, "rb");
				
				fseek(fp, 0L, SEEK_END);
				int size = ftell(fp);
				fseek(fp,0L, SEEK_SET);
				
				char aux[32];
				sprintf(aux, "%d", size);
				
				char *buffer = code(aux, strlen(aux)+1);
				memcpy(s.payload, buffer, strlen(aux)*2);
				s.len = strlen(aux)*2;
				
				res = send_message(&s);
				if (res < 0) {
						perror("[RECEIVER] Send ACK error. Exiting.\n");
						return -1;
				}
				res = recv_message(&r);
				if( res < 0){
					perror("[RECEIVER] Receive error. Exiting.\n");
					return -1;
				}
				while(!feof(fp)){
					char buffer[700];
					int len = fread(buffer,1,700,fp);
					char * buff = code(buffer, len);
					sprintf(s.payload, "%s", buff);
					s.len = len * 2 ;
					res = send_message(&s);
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}
					res = recv_message(&r);
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}
				}
				fclose(fp);
			}
			
			if(strcmp (command, "sn") == 0){
				recv_message(&r);
				int fileDimensions = atoi(correctMessage(r.payload, r.len));
				printf("Dimensiune = %d \n", fileDimensions);
				res = sendACK();
				if( res < 0){
					perror("[RECEIVER] Receive error. Exiting.\n");
					return -1;
				}
				char * snFile = (char *) malloc(255 * sizeof(char));
				strcpy(snFile, "new_");
				strcat(snFile, file);
				FILE *fp = fopen(snFile, "wb");
				
				while(fileDimensions > 0){
					res = recv_message(&r);
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}
					int length = fwrite(correctMessage(r.payload, r.len), 1 , 1400, fp);
					res = sendACK();
					if( res < 0){
						perror("[RECEIVER] Receive error. Exiting.\n");
						return -1;
					}
					fileDimensions -= length;
				}
				fclose(fp);
			}
			
			if(strcmp (command, "exit") == 0){
				break;
			}
			
			if(strcmp(command, "exit") == 0){
				break;
			}
		}
	}
	printf("[RECEIVER] Finished receiving..\n");
	return 0;
}
