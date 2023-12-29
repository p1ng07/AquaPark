# AquaPark

Projeto de sistemas operativos

# Como correr

Terminal de monitor

```bash
$ make init
$ make all
$ ./bin/monitor
```

Terminal do simulador (depois de começar o processo monitor)

```bash
$ ./bin/simulador conf/simulador.conf
```

O ficheiro configuração do simulador deve ter:

- Users iniciais (nome: users)
- Segundos que a simulação deve correr (nome: tempo_simulacao)
- % Chance de ocorrer um acidente quando um user está numa atração (nome: accident_chance)
- % Chance de um user desistir da sua fila de espera (nome: quit_chance)
- Intervalo de micro segundos para criar um novo utilizador no parque (nome: ritmo_chegada)
