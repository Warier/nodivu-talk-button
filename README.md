# Nodivu Botão de fala

![Logo](logo.png)

Plugin nativo leve, gratuito e MIT para o [Nodivu](https://github.com/Warier/nodivu).
Processamento local em CPU, sem GPU, rede ou modelos. Versão **1.0.0**, Windows x64.

## Compatibilidade e instalação

Requer **Nodivu com contrato de plugins host_api 2** (desenvolvimento posterior à 0.2.0).
A release 0.2.0 original não suporta este pacote. O catálogo deve mostrá-lo como incompatível nela.
No host atualizado, abra **Gerenciar plugins → Explorar**, escolha este plugin e instale.
Confirme a confiança no autor, salve seu projeto e feche/abra o Nodivu.
Alternativa: baixe o ZIP em [Releases](https://github.com/Warier/nodivu-talk-button/releases)
e use **Adicionar ZIP local**. Não extraia sobre uma DLL em uso.

## Como usar

1. Ligue microfone → **Botão de fala** → Mixer/saída.
2. Escolha **Segurar para falar** (padrão). Segure o botão com o mouse ou, com ele focado, Espaço/Enter.
3. Solte: o microfone fecha suavemente. Ao perder foco ou ocultar a janela, também fecha.
4. Para falar com um jogo em primeiro plano, use **Alternar** e marque **Aberto** antes de voltar ao jogo.

| Modo | Comportamento |
|---|---|
| Sempre aberto | Passa o áudio continuamente |
| Segurar para falar | Passa enquanto o botão está pressionado e a interface renova o comando |
| Alternar | O controle Aberto liga/desliga a passagem |

A transição padrão de 5 ms evita cortes abruptos. Não há atalho global nesta versão.
O comando de segurar é transitório, nunca salvo no projeto; expira em até 350 ms sem renovação,
mais o próximo bloco de áudio e a rampa de fechamento. Não é uma barreira de privacidade:
**Bypass contorna o bloco e deixa o microfone passar**, assim como um fio paralelo.
O modo Alternar e seu estado Aberto são configurações persistidas; ao reabrir um projeto
com esse modo aberto, o microfone volta a passar conforme salvo.


## Consumo e latência

Algoritmo constante por amostra, sem alocação, locks, arquivos ou rede no processamento.
Não adiciona fila, janela de antecipação nem atraso algorítmico declarado: **0 frames**.
Tempos de detector/rampa são resposta do controle, não um buffer que atrasa todo o áudio.
Classe **leve** é uma declaração do projeto, não garantia de CPU ou latência ponta a ponta.
A latência dos dispositivos/Windows/Nodivu permanece. Instâncias mantêm estados independentes.

## Compilar e testar

Instale Git e Visual Studio 2022 Build Tools com C++ x64 e Windows SDK; PowerShell 7 recomendado.
Não precisa compilar o Nodivu nem instalar Rust, Node ou Python para este plugin.

```powershell
git clone https://github.com/Warier/nodivu-talk-button.git
cd nodivu-talk-button
./scripts/Build.ps1
```

O script compila C++17 com `/O2 /W4 /WX /MT`, executa os testes DSP e gera:

- `dist/package/`: pacote importável com biblioteca CLAP, manifestos, README e licenças.
- `dist/nodivu-talk-button-1.0.0-windows-x64.zip`: mesmo pacote para distribuição.
- `dist/SHA256SUMS.txt`: hash para conferir o ZIP; não equivale à assinatura Authenticode.

## Organização para autores

| Arquivo | Responsabilidade |
|---|---|
| `src/dsp.h` | Algoritmo, parâmetros e estado; independente de CLAP e do host |
| `src/clap_plugin.cpp` | ABI CLAP, lifecycle, validação e tradução de eventos/buffers |
| `plugin.json` | Aparência declarativa no bloco/inspetor; não executa HTML/JS |
| `nodivu-package.json` | Identidade, versão, plataforma e host mínimo |
| `tests/dsp_test.cpp` | Comportamento, estéreo, independência e reset |
| `scripts/Build.ps1` | Build, testes e empacotamento |
| `CONTRACT.md` | Requisitos precisos de áudio, parâmetros, host e distribuição |

[Contrato deste plugin](https://github.com/Warier/nodivu-talk-button/blob/main/CONTRACT.md).
[Contrato de integração do host](https://github.com/Warier/nodivu/blob/main/PLUGIN-NATIVE.md).
O README incluído na loja pertence à mesma versão imutável do ZIP; não é o README futuro da main.

## Licença e contribuição

Código MIT; contribuições por PR são bem-vindas. CLAP 1.2.2 é MIT, com sua licença separada.
Uma DLL nativa executa com os direitos do backend: instale somente pacotes de autores confiáveis.
Mudanças publicadas exigem nova versão; nunca substitua os bytes da release 1.0.0.

