#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "asteroids/game/service/repository/FileRecordRepository.h"

// O TSV de verdade: ida e volta em disco, e o que acontece quando o arquivo nao
// existe ou esta estragado. (As REGRAS de recorde sao testadas sem disco, no
// RecordServiceTest.)

namespace {

class FileRecordRepositoryTest: public ::testing::Test
{
protected:
    std::filesystem::path path;

    void SetUp() override
    {
        path = std::filesystem::temp_directory_path() /
               ("asteroids_records_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) + "_" +
                ::testing::UnitTest::GetInstance()->current_test_info()->name() + ".tsv");
        std::filesystem::remove(path);
    }

    void TearDown() override { std::filesystem::remove(path); }

    void write(const std::string& content) const
    {
        std::ofstream file(path, std::ios::trunc);
        file << content;
    }
};

} // namespace

TEST_F(FileRecordRepositoryTest, MissingFileLoadsAnEmptyList)
{
    ast::FileRecordRepository repository{ path.string() };

    // Primeiro uso do jogo: nao ha arquivo, e isso NAO e um erro.
    EXPECT_TRUE(repository.loadAll().empty());
}

TEST_F(FileRecordRepositoryTest, SavesAndLoadsBackTheSameRecords)
{
    ast::FileRecordRepository repository{ path.string() };

    repository.saveAll({
        ast::Record{ "ASA", 1200, 3, "2026-07-14 10:00" },
        ast::Record{ "BIA", 900, 2, "2026-07-13 22:31" },
    });

    const auto loaded = repository.loadAll();

    ASSERT_EQ(loaded.size(), 2u);
    EXPECT_EQ(loaded[0].name(), "ASA");
    EXPECT_EQ(loaded[0].score(), 1200);
    EXPECT_EQ(loaded[0].wave(), 3u);
    EXPECT_EQ(loaded[0].playedAt(), "2026-07-14 10:00");
    EXPECT_EQ(loaded[1].name(), "BIA");
}

TEST_F(FileRecordRepositoryTest, SavingReplacesTheWholeFile)
{
    ast::FileRecordRepository repository{ path.string() };

    repository.saveAll({ ast::Record{ "VELHO", 100, 1, "2026-07-01 09:00" } });
    repository.saveAll({ ast::Record{ "NOVO", 200, 1, "2026-07-02 09:00" } });

    const auto loaded = repository.loadAll();

    ASSERT_EQ(loaded.size(), 1u) << "o arquivo e reescrito, nao acrescentado";
    EXPECT_EQ(loaded[0].name(), "NOVO");
}

TEST_F(FileRecordRepositoryTest, BrokenLinesAreSkippedAndTheRestSurvives)
{
    // Arquivo mexido a mao: uma linha sem campos suficientes e outra com um
    // numero invalido. As boas continuam valendo — placar corrompido nao pode
    // custar a tabela inteira.
    write("BOM\t500\t2\t2026-07-14 10:00\n"
          "LINHA_SEM_CAMPOS\n"
          "RUIM\tabc\t1\t2026-07-14 10:00\n"
          "OUTRO\t300\t1\t2026-07-14 11:00\n");

    ast::FileRecordRepository repository{ path.string() };
    const auto                loaded = repository.loadAll();

    ASSERT_EQ(loaded.size(), 2u);
    EXPECT_EQ(loaded[0].name(), "BOM");
    EXPECT_EQ(loaded[1].name(), "OUTRO");
}
