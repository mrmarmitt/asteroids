# Asteroids — dominio C++ puro + testes (task 02).
#
# Mesmo desenho dos irmaos 8puzzle/spaceinvaders: a lib do jogo (dominio +
# maquina de estados) compila sem NENHUMA plataforma; a plataforma The-Forge
# builda a parte via MSBuild (src/platform/theforge/PC_VS2019) contra a arvore
# do The-Forge. A cengine (via FetchContent) exige CMake >= 3.23.
cmake_minimum_required(VERSION 3.23)

project(Asteroids)

# A cengine exige C++23 e propaga o padrao via target_compile_features PUBLIC;
# alinhamos o projeto ao mesmo padrao.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ASTEROIDS_BUILD_TESTS "Build the test suite" ON)

# -----------------------------
# Dependencia: cengine 0.8.0 — o asteroids e um consumidor de validacao dela
# (ADR 0003: os jogos anteriores ficaram estacionados na 0.5.0). O dominio usa
# routing (GameRouter herda de FlowRouter, estados implementam IState) e
# collision2d (a deteccao circulo x circulo; o wrap-around da arena fica AQUI,
# no jogo — a engine nao opina sobre o formato do mundo).
# -----------------------------
include(FetchContent)

FetchContent_Declare(
  cengine
  GIT_REPOSITORY https://github.com/cengine-dev/cengine.git
  GIT_TAG 0.8.0
)

# Sem a suite de testes da cengine (evita fetch do GoogleTest dela).
set(CENGINE_BUILD_TESTS OFF)
FetchContent_MakeAvailable(cengine)

# -----------------------------
# asteroids_lib — o dominio do jogo, linkavel pela suite de testes.
# A plataforma theforge fica fora (buildada via MSBuild).
# -----------------------------
file(GLOB_RECURSE GAME_SOURCES
        "src/asteroids/*.cpp" "src/asteroids/*.h"
)

add_library(asteroids_lib STATIC ${GAME_SOURCES})
target_include_directories(asteroids_lib PUBLIC src)
target_link_libraries(asteroids_lib PUBLIC
    cengine::core
    cengine::routing
    cengine::collision2d
)

# -----------------------------
# Testes (GoogleTest via CTest)
# -----------------------------
if(ASTEROIDS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
