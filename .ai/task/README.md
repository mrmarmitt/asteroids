# Plano de trabalho - Asteroids

Terceiro jogo do ecossistema cengine. Alem de ser um jogo novo, ele e o
consumidor de validacao da cengine 0.6.0 e do platform-theforge-common 0.1.0
(os jogos anteriores foram congelados — ADR 0003 da cengine).

## Indice

| # | Task | Status | Categoria |
|---|------|--------|-----------|
| 01 | [Bootstrap do casco](01-bootstrap-casco.md) | in-progress | Plataforma |

## Backlog (tasks a abrir quando chegarem)

Sequencia prevista, um degrau validado por vez (mesma disciplina dos jogos
anteriores):

- **02 — Dominio: World da nave** (C++ puro + testes CMake): nave com
  rotacao/thrust/inercia, wrap-around da arena, tiros com cooldown.
  Pre-requisito de desenho: rotacao no `forgesprite` (o batcher 0.1.0 nao
  rotaciona — necessidade prevista na task 01 do platform-theforge-common,
  "salvo se o consumidor real exigir"; o asteroids exige).
- **03 — Asteroides + colisao**: fragmentacao (grande -> medio -> pequeno),
  colisao tiro x asteroide e nave x asteroide. Gate da task 17 da cengine
  (`cengine::collision2d`) dispara aqui — decidir AABB x circulo com o
  consumidor na mao.
- **04 — Pontuacao, vidas e gameOver**: fluxo completo (estado gameOver
  entra na maquina), UI de HUD.
- **05 — Recordes** (politica do jogo, padrao TSV dos irmaos — NAO promove
  para a engine, ADR 0002).

## Regra pratica

Dominio em `src/asteroids/` sem nenhum include do The-Forge; plataforma em
`src/platform/theforge/`. Toda mexida na cengine ou no common e validada por
este jogo antes de taggear.
