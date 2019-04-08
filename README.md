![](https://img.clasf.com.br/2016/11/24/Ciclocomputador-GPS-Garmin-Edge-200-Novo-20161124214433.jpg)

- Atualize o `SAME70-examples`
    - vai ter um novo exemplo: `Perifericos-uc/RTT`
- Fork no github o repositório `github.com/insper/AV1-Embarcados`
- Clone o seu `fork`
- Atualize o arquivo `ALUNO.json` com seu nome e email e faça um **commit** imediatamente.
- Preencha o [`FORMS`](https://forms.gle/22FHN1yKEwQJ9bJi8)
    - https://forms.gle/22FHN1yKEwQJ9bJi8
    
- **A partir desse ponto deve-se desligar a internet**
    - Coloque o PC em modo avião!
    - Consulte somente seus arquivos!
    - Consulta a colegas/ internet constituirão violações ao Código de Ética e de Conduta e acarretarão sanções nele previstas. Faça o seu trabalho de maneira ética!

- **A cada 15 minutos você deverá fazer um commit no seu código!**
    - Códigos que não tiverem commit a cada 15min ou que mudarem drasticamente entre os commits serão descartados (conceito I) !!
    - Você deve inserir mensagens condizentes nos commits!

- Ao terminar a avaliação, religue a internet e faça o `push` do seu repositório!
    - Chame o professor quando chegar nesse momento.

Essa avaliação é composta de duas partes: `Quiz` e `Prática`. Sendo reservados **40min** para o `Quiz` e mais **3h00** para a parte `Prática`. 

## Rubrica

- A (**obrigatórios** + 1 extra)
    - **Desliga LCD quando em idle por mais de 20 segundos**
    - **Para contagem de tempo quando em idle**
    - **Cria `.h` e `.c` para organizar o código (com funções referentes ao computador)**
    - Exibe seta que indica se velocidade está aumentando ou diminuindo
    - Substitui botão por touch screen
    - Adiciona velocidade máxima a interface
- B 
    - Faz uso de interrupção sempre que possível.
    - Adiciona um botão de PAUSE/START
        - Para/Reinicia contagem da distância e do tempo
    - Cria funções que auxiliam a organização do código
    - Minimiza a permanência nas interrupção
    - Entra em sleepmode
- C
    - Calcula e exibe no LCD a velocidade instantânea (km/h) - a cada 4s
    - Calcula e exibe no LCD a distância total (m) - a cada 4s
    - Exibe no LCD tempo total (HH:MM:SS) - a cada segundo
    - Lê sensor de rotação (simulado via botão externo)
    - Acerta o calculo da velocidade e distância
- D
    - Dois itens de C faltando/errado
- I
    - Três ou mais itens de C faltando/errado

## Descrição

Você deve projetar um protótipo de um `ciclocomputador` com interface LCD. O ciclocomputador é um sistema embarcado instalado na bicicleta que fornece informações ao ciclista tais como: velocidade; distância; tempo. 

O `ciclocomputador` funciona via a leitura de um sensor magnético instalado no garfo frontal da bicicleta e que gera um pulso elétrico sempre que a roda completa uma revolução completa. Através desse pulso é possível extrair a velocidade angular e por consequência a velocidade da bicicleta, distância percorrida.

A entrega deve ser um firmware que exibe no LCD as seguintes informações:

1. Velocidade instantânea 
    - atualizada a cada **4 segundos**
1. Distância total (m)
    - atualizada a cada **4 segundos**
1. Tempo total do trecho (HH:MM:SS)
    - atualizado a **cada segundo**

- Utilize um botão externo da placa para simular o sensor hall (roda girando)
- Considere uma bicicleta quadro 26", `d = 650mm`


# Dicas de implementação

A seguir algumas dicas de implementação:

## Projeto base - LCD

Utilize como projeto base o exemplo do `LCD-maXTouch-New-Fonts/` que já está na pasta do repositório da avaliação.

## Velocidade

A velocidade da bicicleta (v) é decorrente da velocidade angular (w) de sua roda, sendo calculado por: `v = w*r [m/s]`.

Existem duas maneiras de se calcular a velocidade angular: 

- mede-se o tempo (`t`) entre dois pulsos e a partir da frequência (`f=1/t`) calcula-se `w`: `w = 2*pi*f [rad/s]`
- acumula-se pulsos (`N`) em um determinado tempo (`dT`): `w = 2*pi*N/dT`

Como só conseguimos medir um pulso por rotação, é necessário que esse `dT` seja: suficiente alto para medirmos uma velocidade relativamente baixa, mas não pode ser tão elevado, caso contrário teremos uma taxa de atualização da velocidade muito lenta.

Nesses dois casos não podemos utilizar o TC para medirmos a frequência (`f`) ou gerar o `dT` pois a menor frequência na qual o TC operar/medir é de 0.5Hz (o que daria uma velocidade mínima de 3.3Km/h).

## Distância

A distância (`d`) percorrida pela bicicleta é: `d = 2*pi*r*N [m]`.

### RTT

O **Real Time Time** (RTT) é um timer similar ao TC, mas bem mais simples. Serve apenas para contar `clocks`. Possui um registrador de 32 bits, ao contrário do TC que o registrador é de 12 bits. O RTT pode ser alimentado por dois clocks distintos. O que permite gerarmos interrupções com grandes períodos de tempo (vamos usar para medir `f` ou  gerar o `dT`)!

![](rtt.png)

O RTT pode gerar interrupção por duas fontes distintas: `Alarme` ou `Mudança no valor do relógio`. O Alarme funciona similar ao alarme do RTC, podemos especificar um valor e quando o contador chegar nesse valor, gera uma interrupção, ou podemos gerar uma interrupção toda vez que o valor do timer mudar (tick). Isso vai depender da frequência na qual ele foi configurado (funcionando similar ao TC).

Utilize o exemplo disponibilizado em `SAME70-Examples/Perifericos-uC/RTT` que inicializa e configura o RTT do SAME70 para resolver a questão do calculo de velocidade.

## Cadência

Cadência é a medida de rotações por minuto do pedal da bicicleta.

## LCD

Não se apegue muito a estética, primeiro exiba corretamente os valores e depois se tiver tempo você tenta deixar funcional.



