# Atividade 2 - Computação Gráfica

**Nome** : Lucas Silva Amorim

**RA** : 11201720968

**Live Demo**: https://lucas170198.github.io/first-person-env/firstpersonenv/

## First Person Env
### Descrição da Aplicação
Desenvolvimento de engine para game em primeira pessoa. Foi utilizada a técnica de câmera lookat como sendo o "personagem" controlado pelo player. Foram implementados os comandos de: PULAR, ANDAR, CORRER e VIRAR A CÂMERA. O player está limitado a área que possui um piso.

### Técnicas Utilizadas
**Dolly** : Calcula o vetor foward (através de uma subtração entre os vetores **eye** e **at**) e se move em direção do vetor de acordo com a variável `m_dollySpeed` (a velocidade é aumentado se o player segurar a tecla **CTRL** para correr).

**Truck** : Calcula o vetor left (através de um produto vetorial entre **up** e **forward**) e se move em direção do vetor de acordo com a variável `m_truckSpeed`.

**Pan** : Camera rotaciona em torno do eixo **y** dando a impreção que o player está olhando a sua volta.

**Jump** : Se movimenta na direção do vetor **up**. Quando atinge uma certa altura (que representa o tamanho do pulo), a variável `m_jumpSpeed` recebe um valor negativo, assim o player começa a cair.

### Controles

JUMP -> SPACE
FOWARD -> UP/DOWN OR W/S
TRUCK -> Q/E
PAN -> A/D OR LEFT/RIGHT
RUN -> CRTL + move

## Missing aspects
- Implementar sistema de combate em primeira pessoa.
- Implementar IA para NPC's
- Tentei implementar um sistema de colisão, porém no momento não está funcionando. A ideia era projetar cada objeto no plano do mundo, e comparar com a projeção da camêra para verificar a colisão da camera com o objeto. A implementação está na função `OpenGLWindow::checkForColisions`.
