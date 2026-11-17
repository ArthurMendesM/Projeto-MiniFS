cmake_minimum_required(VERSION 3.10)
project(MiniFS C)

set(CMAKE_C_STANDARD 11)

# 1. Executável principal integrado (Entrega final do grupo)
add_executable(MiniFS
        main.c
        disco.c
        disco.h
        inode.c
        inode.h
        arquivos.c
        arquivos.h
)

# 2. Executável de teste do Estudante 1
add_executable(teste_est1
        mainTesteEstudante1.c
        disco.c
        disco.h
)

# 3. Executável de teste do Estudante 2
add_executable(teste_est2
        "main TesteEstudante2.c"
        disco.c
        disco.h
        inode.c
        inode.h
)
