# TRABALHO PRÁTICO DE REDES DE COMPUTADORES
João Antônio, Lucas Costa, Lucas Emanuel

## SERVIDOR WEB - TÉCNICAS DISTINTAS

### TÓPICO 1

* Compilação
```
gcc servidor_iterativo.c -o servidor_iterativo -lws2_32
```
* Execução
```
./servidor [porta]
```

### TÓPICO 2

* Compilação
```
gcc servidor_thread.c -o servidor_thread -lws2_32
```
* Execução
```
./servidor [porta]
```

### TÓPICO 3

* Compilação

```
gcc -o server server.c task_queue.c -pthread
```
* Execução
```
./server
```

### TÓPICO 4

* Compilação

```
gcc servidor.c -o servidor
```
* Execução
```
./servidor
```

### TESTE DOS SERVIDORES

```
> curl http://localhost:[porta]
```
```
> telnet localhost [porta]
```



