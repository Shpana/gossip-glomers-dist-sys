# gossip-glomers-dist-sys

Решения челенжда по распределенным системам [Gossip Glomers](https://fly.io/dist-sys/) от компании Fly.io.

Тестирование решений производится с помощью фреймворка [maelstrom](https://github.com/jepsen-io/maelstrom) --- небольшой утилиты, эмулирующей работу сети с разнообразными условиями.
Для взаимодействия с фреймворком была написана обертка на C++ maelstrom-cpp,
подробнее об использовании и о внутреннем устройстве можно найти в [maelstrom-cpp](/maelstrom-cpp/README.md).

Здесь приведены решения задач
1. [echo](tasks/echo/): каждая нода должна возвращать только что полученное сообщение.
2. [unique ids](tasks/unique_ids/): totally available система генерации глобально уникальных идентификаторов. 
3. [broadcast](tasks/broadcast/): система широковещательной рассылки, которая обменивается сообщениями между всеми узлами кластера.
4. [gcounter](tasks/gcounter/): распределенный  eventually-consistent stateless grow-only счетчик, построенный поверх sequentially-consistent key/value хранилища.
5. [kafka](tasks/kafka/): реплицированная служба логов, похожая на очень урезанную версию Kafka. 