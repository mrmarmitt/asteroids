# 06 - Renderizacao de verdade: wireframe vetorial

- **Status:** done (2026-07-14 — validado jogando pelo usuario)
- **Prioridade:** media - o jogo estava inteiro, mas desenhado com letras.
- **Categoria:** Plataforma
- **Depende de:** platform-theforge-common >= 0.2.0 (`forgeline`).

## Contexto

Desde a task 02 o jogo se desenha com TEXTO: a nave era um triangulo de pontos,
as rochas eram aneis de pontos. Funcionava para validar fisica e colisao, e era
feio de proposito — a decisao do renderizador foi ADIADA ali, com o porque
registrado: escolher antes de o jogo existir seria decidir sem o consumidor na
mao (ADR 0002 da cengine, aplicada ao common).

Agora o jogo esta inteiro (nave, rochas, colisao, placar, recordes). A decisao
podia ser tomada com tudo a vista.

## Decisao: wireframe (linhas), e o `forgesprite` NAO ganha rotacao

O backlog previa "rotacao no forgesprite". Descartado. Duas razoes, a segunda
decisiva:

1. **Fidelidade.** Asteroids e um arcade VETORIAL: a tela original e desenhada a
   linha. Wireframe nao aproxima o original — ele E o original.
2. **Linha dispensa ATLAS.** Nao ha arte para produzir, versionar ou carregar:
   um poligono girado e uma lista de pontos girados. Sprite rotacionado exigiria
   um atlas que nao existe E traria o serrilhado/pivot do bitmap girado.

Virou a task 02 do platform-theforge-common (`forgeline`, 0.2.0).

## Escopo

1. Common 0.2.0: `forgeline` (batcher de linhas opt-in, sem atlas e sem SRT).
2. `main_theforge.cpp`: `windowDesc.lines.enabled = true` (o batcher de SPRITES
   segue desligado — e continua assim: este jogo nao tem atlas).
3. `ForgeGameScene`: nave (triangulo com a popa reentrante do arcade), chama do
   motor, rochas (poligonos irregulares) e tiros. HUD segue em texto, por cima
   (o `drawText` da flush no lote de linhas).

## Decisoes de desenho

**A silhueta da rocha e assunto da CENA, nao da simulacao.** O `World` diz onde
a rocha esta e que tamanho tem; os "amassados" do poligono vem de um ruido
deterministico por (indice, vertice) — a mesma rocha tem sempre a mesma
silhueta, sem o dominio guardar um unico dado de desenho e sem a rocha tremer
entre quadros.

**O corpo que cruza a borda aparece INTEIRO dos dois lados.** A arena e um toro;
desenhar so o poligono no centro deixaria a nave cortada pela metade na borda. A
cena desenha ate 4 copias deslocadas (+-arena em X e Y) e deixa o recorte da
tela cuidar do resto — sao ~3 linhas de codigo e resolvem o artefato visual mais
obvio do wrap.

## Criterios de Aceite

- [x] Build MSBuild verde (Release|x64); FSL compila os shaders de linha.
- [x] Log limpo (pipeline sem SRT aceito pelo D3D12).
- [x] Nenhum atlas, nenhuma arte: o batcher de sprites continua DESLIGADO.
- [x] Validacao jogavel: nave e rochas em wireframe, girando de verdade; a chama
      aparece no thrust; objetos na borda aparecem inteiros dos dois lados; HUD
      por cima. (Validado em 2026-07-14.)
