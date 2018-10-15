# Makefile (règles implicites)

CC = gcc
CFLAGS = -Wall -Wextra -Werror

tout: echoclient echoserveur EchoClient.class

EchoClient.class:EchoCLient.java
	javac EchoCLient.java

clean:
	rm *.o *.class