# realtime-pong


## Requirements 

### Xenomai

### SDL interface
sudo apt-get install libsdl1.2-dev

## Execution

make
./pong [name_user1] [name_user2 OR 'computer'] 




resource interface SDL: https://github.com/flightcrank/pong


gcc -o pong pong.c `sdl-config --cflags --libs`

install SDL sudo apt-get install libsdl1.2-dev


tarefa cinemática

ficheiro de caracteristicas/parametros

bola -> posicao x ; posicao y ; velocidade X max; velocidade Y max ; velocidade X min; velocidade Y min ;
barra -> posicao y
campo -> x, y


tarefa input barra


exemplos xenomai: http://www.cs.ru.nl/lab/xenomai/exercises
https://github.com/yunusdawji/xenomai-examples

