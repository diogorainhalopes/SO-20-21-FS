# SO-20-21-FS
_IST - Projeto de Sistemas Operativos - 20/21 LEIC-A_

  *Visão global do projeto
O objetivo final do projeto é desenvolver um sistema de ficheiros (File System, FS) em modo utilizador
(user-level) e que mantém os seus conteúdos em memória primária, chamado TecnicoFS.*

Os FS em modo utilizador são uma alternativa que tem vindo a ganhar relevância recentemente, já
que permitem o desenvolvimento rápido de FS facilmente portáveis e com forte isolamento de falhas,
tal como será discutido nas aulas teóricas durante o semestre. 
  Num FS em modo utilizador, as
funcionalidades do FS são oferecidas num processo servidor (que, naturalmente, corre em modo
utilizador). 
  Outros processos podem chamar funções do FS através de pedidos ao núcleo do Sistema
Operativo, que, por sua vez, encaminha esses pedidos ao processo servidor do FS através de um canal
de comunicação estabelecido com este. Posteriormente, o retorno da função é devolvido ao cliente
invocado pela via inversa.
  Ao contrário de FS tradicionais, que guardam a informação em blocos (e.g., num disco magnético ou
SSD), o TecnicoFS é desenhado para armazenar o conteúdo dos seus ficheiros e diretorias em memória
primária. Pode, por exemplo, ser usado para manter um FS temporário, não persistente, em memória
DRAM, beneficiando assim do melhor desempenho desta memória, em comparação com disco/SSD.
  Pode também aproveitar as novas tecnologias de memória persistente quando usado em máquinas
em que estas estejam disponíveis (como, por exemplo, os recentes DIMMs com tecnologia Intel
Optane DC).


PS: No teste prático foi adicionada a possibilidade de no lançamento do server (E_3/Server/main.c) 
existir um quarto argumento que faz com que o server entre em sleep pela quantidade de tempo dada no input.
Retirar caso surjam problemas. (SO/E_3/Server/main.c lines 170-176)
