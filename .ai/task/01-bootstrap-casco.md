# 01 - Bootstrap do casco (The-Forge + cengine 0.6.0 + common 0.1.0)

- **Status:** done (2026-07-14 — validado jogando pelo usuario)
- **Prioridade:** alta - nada existe sem o casco; e tambem o aceite real
  pendente de dois releases (cengine 0.6.0 e platform-theforge-common 0.1.0).
- **Categoria:** Plataforma
- **Depende de:** checkouts irmaos The-Forge (Unit_Tests buildada), cengine
  >= 0.6.0, platform-theforge-common >= 0.1.0.

## Contexto

O asteroids nasce como o primeiro consumidor do novo desenho do ecossistema:

- `EngineManager::owned(window, game)` — task 21 da cengine (opcao B);
- `GameRouter` herdando de `cengine::routing::FlowRouter<StateGameFlow>` —
  task 19 da cengine (nenhuma mecanica duplicada, so vocabulario);
- `TheForgeWindowManager` do common configurado por `TheForgeWindowDesc` —
  task 01 do platform-theforge-common, incluindo o batcher DESLIGADO
  (`atlasPath` nulo): o casco e so texto, validando o caminho opt-in.

## Objetivo

Janela abrindo com o fluxo splash -> menu -> game(placeholder) -> exit
rodando de ponta a ponta, dirigido pela cengine em modo proprio.

O placeholder do game valida o input segurado do common (`heldAxis`): uma
"nave" de texto move com as setas; ESC volta ao menu.

## Escopo

1. Dominio minimo: `GameRouter` (FlowRouter + menu/game/exit),
   `StateGameFlow`, estados initial/menu/game/exit (`exit` casa com
   `cengine::routing::kExitStateCode`).
2. Cenas Forge: splash, menu (JOGAR/SAIR), game placeholder, exit.
3. `main_theforge.cpp`: subsistemas de processo + composicao + owned().
4. `AsteroidsForge.vcxproj/.sln` consumindo os tres repos irmaos; FSL
   apontando para a `Shaders.list` do common; PathStatement/gpu.cfg proprios.

## Fora do Escopo

- World/simulacao da nave (task 02 — exige rotacao no forgesprite).
- gameOver/pontuacao/recordes (tasks 04/05).
- Atlas/sprites (entra quando a task 02 definir a arte).

## Criterios de Aceite

- [x] Build MSBuild verde com os tres repos irmaos (Release|x64 — o
      The-Forge desta maquina so tem as libs Release buildadas).
- [x] Fluxo completo jogavel: splash -> menu -> game -> menu -> exit, janela
      fecha limpa (cleanup sem crash/leak report do The-Forge).
- [x] Nave-texto move com setas SEGURADAS no placeholder (heldAxis do
      common validado).
- [x] Nenhum include do The-Forge em `src/asteroids/` (dominio puro).

Validacao jogavel feita pelo usuario em 2026-07-14. Este aceite tambem fecha
o aceite real pendente da cengine 0.6.0 (owned() + FlowRouter em consumidor
vivo) e do platform-theforge-common 0.1.0 (primeiro consumidor visual).
