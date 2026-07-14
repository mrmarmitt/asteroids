#include <gtest/gtest.h>

#include <memory>

#include "asteroids/game/service/RecordService.h"
#include "asteroids/game/service/repository/RecordRepository.h"

// As REGRAS de recorde, exercitadas com um repositorio de mentira: ranking,
// corte do top-N e o que conta como recorde novo. Nada de disco aqui — o teste
// do TSV de verdade vive no FileRecordRepositoryTest.

namespace {

class FakeRecordRepository final: public ast::RecordRepository
{
public:
    std::vector<ast::Record> stored;
    int                      saveCount = 0;

    [[nodiscard]] std::vector<ast::Record> loadAll() override { return stored; }

    void saveAll(const std::vector<ast::Record>& records) override
    {
        stored = records;
        ++saveCount;
    }
};

ast::Record record(const int score, const std::string& name = "P1", const uint32_t wave = 1)
{
    return ast::Record{ name, score, wave, "2026-07-14 10:00" };
}

// Um repositorio ja cheio (kMaxRecords), com pontuacoes 1000, 2000, ... — o
// ultimo colocado tem 1000.
std::shared_ptr<FakeRecordRepository> fullRepository()
{
    auto repository = std::make_shared<FakeRecordRepository>();
    for (size_t i = 0; i < ast::RecordService::kMaxRecords; ++i)
    {
        repository->stored.push_back(record(1000 * static_cast<int>(i + 1)));
    }
    return repository;
}

} // namespace

TEST(RecordServiceTest, StartsEmptyWhenNothingWasPersisted)
{
    const ast::RecordService service{ std::make_shared<FakeRecordRepository>() };

    EXPECT_TRUE(service.listByScore().empty());
}

TEST(RecordServiceTest, RanksByScoreDescending)
{
    const auto repository = std::make_shared<FakeRecordRepository>();
    ast::RecordService service{ repository };

    service.addRecord(record(500, "MEIO"));
    service.addRecord(record(900, "TOPO"));
    service.addRecord(record(100, "FUNDO"));

    const auto& ranked = service.listByScore();
    ASSERT_EQ(ranked.size(), 3u);
    EXPECT_EQ(ranked[0].name(), "TOPO");
    EXPECT_EQ(ranked[1].name(), "MEIO");
    EXPECT_EQ(ranked[2].name(), "FUNDO");
}

TEST(RecordServiceTest, PersistsOnEveryNewRecord)
{
    const auto         repository = std::make_shared<FakeRecordRepository>();
    ast::RecordService service{ repository };

    service.addRecord(record(500));

    EXPECT_EQ(repository->saveCount, 1);
    ASSERT_EQ(repository->stored.size(), 1u);
    EXPECT_EQ(repository->stored[0].score(), 500);
}

TEST(RecordServiceTest, KeepsOnlyTheTopNAndDoesNotPersistTheRest)
{
    const auto         repository = fullRepository(); // ja cheio; ultimo tem 1000
    ast::RecordService service{ repository };

    service.addRecord(record(50, "FRACO"));

    // A tabela nao cresce...
    EXPECT_EQ(service.listByScore().size(), ast::RecordService::kMaxRecords);
    // ...e o que nao entrou no top-N NAO vai para o arquivo (senao ele cresceria
    // para sempre, como no spaceinvaders).
    EXPECT_EQ(repository->stored.size(), ast::RecordService::kMaxRecords);
    for (const auto& stored : repository->stored)
    {
        EXPECT_NE(stored.name(), "FRACO");
    }
}

TEST(RecordServiceTest, GoodScoreEvictsTheLastPlace)
{
    const auto         repository = fullRepository();
    ast::RecordService service{ repository };

    service.addRecord(record(99999, "CAMPEAO"));

    const auto& ranked = service.listByScore();
    ASSERT_EQ(ranked.size(), ast::RecordService::kMaxRecords);
    EXPECT_EQ(ranked.front().name(), "CAMPEAO");
    EXPECT_EQ(ranked.back().score(), 2000) << "o antigo ultimo colocado (1000) foi expulso";
}

TEST(RecordServiceTest, AnyScoreIsARecordWhileThereIsRoom)
{
    const ast::RecordService service{ std::make_shared<FakeRecordRepository>() };

    EXPECT_TRUE(service.isNewRecord(1)) << "tabela vazia: ate 1 ponto entra";
}

TEST(RecordServiceTest, WithAFullTableOnlyBeatingTheLastPlaceCounts)
{
    const ast::RecordService service{ fullRepository() }; // ultimo colocado: 1000

    EXPECT_TRUE(service.isNewRecord(1001));
    EXPECT_FALSE(service.isNewRecord(1000)) << "empatar com o ultimo nao entra";
    EXPECT_FALSE(service.isNewRecord(999));
}

TEST(RecordServiceTest, NormalizesAFileThatCameUnsortedOrTooBig)
{
    const auto repository = std::make_shared<FakeRecordRepository>();

    // Arquivo editado a mao: desordenado e com mais linhas que o teto.
    for (size_t i = 0; i < ast::RecordService::kMaxRecords + 5; ++i)
    {
        repository->stored.push_back(record(static_cast<int>(i) * 7 % 500));
    }

    const ast::RecordService service{ repository };

    const auto& ranked = service.listByScore();
    ASSERT_EQ(ranked.size(), ast::RecordService::kMaxRecords);
    EXPECT_TRUE(std::is_sorted(ranked.begin(), ranked.end(),
                               [](const ast::Record& a, const ast::Record& b) { return a.scoresHigherThan(b); }));
}

TEST(RecordServiceTest, SanitizesTheNameThatWouldBreakTheTsv)
{
    // Um tab no nome partiria a linha em campos errados na proxima leitura.
    EXPECT_EQ(ast::RecordService::sanitizeName("A\tB"), "AB");
    EXPECT_EQ(ast::RecordService::sanitizeName("A\nB"), "AB");
}

TEST(RecordServiceTest, TrimsLongNamesAndNamesTheNameless)
{
    EXPECT_EQ(ast::RecordService::sanitizeName("NOMEMUITOLONGO").size(), ast::RecordService::kMaxNameLength);
    EXPECT_EQ(ast::RecordService::sanitizeName(""), "ANONIMO");
    EXPECT_EQ(ast::RecordService::sanitizeName("\t\t"), "ANONIMO");
}

TEST(RecordServiceTest, StoredNameIsTheSanitizedOne)
{
    const auto         repository = std::make_shared<FakeRecordRepository>();
    ast::RecordService service{ repository };

    service.addRecord(record(100, "COM\tTAB"));

    ASSERT_EQ(repository->stored.size(), 1u);
    EXPECT_EQ(repository->stored[0].name(), "COMTAB");
}
