# Contrato — org.nodivu.talk-button / 1.0.0

## Identidade e distribuição

Descriptor CLAP e plugin.json usam `org.nodivu.talk-button`. Host API **2**, win32-x64,
CLAP 1.2.2, biblioteca `talk-button.clap`. Um plugin por pacote. ID estável; versão semântica
numérica no envelope. Inclua todos os arquivos de `dist/package`, sem build scripts,
segredos ou fonte como substituto da DLL. Licença MIT e licença CLAP acompanham o ZIP.

## Áudio e lifecycle

48.000 Hz, float32 planar estéreo, 1..480 frames por chamada, uma saída.
Uma entrada: índice 0/ID CLAP 10 é principal.
Saída índice 0/ID CLAP 20. IDs de fios do Nodivu são índices estáveis **0 e 1**,
independentes dos IDs CLAP 10/11/20. Portas não conectadas recebem zeros.
`in_place_pair=CLAP_INVALID_ID`: buffers distintos, entrada nunca escrita.
Latência constante 0 após activate. Não muda portas/latência durante ativação.
Initialize/activate/start/process/stop/deactivate/destroy seguem as threads CLAP.
Alocação somente em create; delete somente em destroy, fora do áudio.

Não há estado global mutável nem compartilhamento DSP entre instâncias. Reset limpa
histórico/envelope/rampa, preserva parâmetros conforme contrato CLAP; o host republica controles.
Eventos aceitos: `CLAP_EVENT_PARAM_VALUE`, até 32 por chamada, ordenados por frame/time.
Somente valores finitos, dentro dos limites; enumerados são inteiros. Parâmetros consultados
por get_value usam atomics. O host não chama flush simultaneamente com process para a mesma instância.
Sem logs, I/O, locks, alocação ou espera em process. Entradas não finitas retornam erro;
o host silencia a saída e preserva a causa. Nunca misturar a referência na música.

## Parâmetros estáveis

| ID | Tipo | Faixa | Padrão |
|---|---|---|---|
|0|inteiro/modo|0 sempre aberto, 1 segurar, 2 alternar|1|
|1|inteiro/bool|0..1, estado do modo alternar|0|
|2|inteiro/bool momentâneo|0..1, gesto fornecido pelo host|0|
|3|contínuo/ms|1..50|5|

O widget `momentary` vincula ID 2 à API `plugin.gesture` do host. Renova a cada 100 ms;
o host encerra a concessão em 350 ms sem renovação. O renderer envia release em pointerup,
pointercancel, lostpointercapture, blur, ocultação e remoção do controle. O host mantém
esse valor fora do documento, exige 0 no grafo e aplica o comando somente ao UUID correto.
Não existe captura de teclado global dentro da DLL. Outro host CLAP que enviar parâmetro 2
sem o mecanismo de concessão assume a responsabilidade de liberá-lo; a garantia de timeout
é do Nodivu host_api 2, não uma extensão do padrão CLAP.

## Frontend, atualização e loja

Controles definidos em plugin.json; limites e defaults vêm do CLAP. Texto português UTF-8.
O plugin não carrega scripts de interface. O autor pode alterar grupos/ajuda/controles rápidos
sem mudar DSP, mas bytes distribuídos novos exigem versão nova.
O catálogo assinado deve declarar host_api 2, resource_tier light, publisher_id nodivu,
ZIP/README/logo por versão e hashes TUF. Selo oficial vem do catálogo confiável, não do manifesto.
Instalação/atualização só ativa no próximo início. Até oito tipos e oito nós externos no host atual.
Não há sandbox de DLL, atalho global nem ativação quente nesta versão.
