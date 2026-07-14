# Configuracao dos testes (mesmo padrao dos irmaos e da cengine)

# -------------------------------------------------------------
# 1. GoogleTest via FetchContent
# -------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

# -------------------------------------------------------------
# 2. Executavel de testes — linka a lib do jogo
# -------------------------------------------------------------
add_executable(asteroids_tests
    "${CMAKE_CURRENT_SOURCE_DIR}/asteroids/game/WorldTest.cpp"
)

target_link_libraries(asteroids_tests
  PRIVATE
  asteroids_lib
  gtest_main
)

# -------------------------------------------------------------
# 3. Registrar no CTest
# -------------------------------------------------------------
include(GoogleTest)

gtest_discover_tests(asteroids_tests)
