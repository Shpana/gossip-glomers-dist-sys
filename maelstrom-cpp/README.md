# maelstrom-cpp

Библиотека для асинхронной обработки maelstrom-запросов. 
Для реализации конкуретности используется довольно забавная библиотека [YACLib](https://github.com/YACLib/YACLib).

### Node

Для обработки входящих запросов maelstrom используется `maelstrom::Node<State>`.

```cpp
maelstrom::Node<State> node;
node.Add<DummyHandler>();
node.Add<DummyWorker>(100ms);
node.run()
```

### Routines

Входящие запросы можно обрабатывать с помощью handers.
Для того, чтобы указать какого типа запросы будут обрабатываться данным handle нужно указать обязательное статическое поле `kType`.
Метод `Handle` вызывается каждый раз, когда от maelstrom пришел очередной запрос.
```cpp
class DummyHandler final : public maelstrom::HandlerBase<State> {
public:
  // Required static field that specifies which types of requests 
  // will be handled by this handler
  static constexpr std::string_view kType = "dummy";

  yaclib::Future<Response> Handle(maelstrom::Network::Session session, 
                                  maelstrom::Request request) override {
    co_return std::move(request).ToResponse();
  }
};
```

Для выполнения background-задач предназначены workers.
Для каждого worker также нужно указать статическое поле `kType`.
Метод `Process` будет вызываться раз в указаное, при конструировании объекта worker'а, время.

```cpp
class DummyWorker final : public maelstrom::WorkerBase<State> {
public:
  // Required static field 
  static constexpr std::string_view kType = "echo";

  yaclib::Future<> Process(maelstrom::Network::Session session) override {
    co_return {};
  }
};
```

### Shared state

Для хранения общего состояния, разделеяемого между всеми handler'ами и worker'ами используется определенный пользователем тип `State`, он передается шаблонным параметром при создании `Node<State>` и указывается при наследовании от `HandlerBase<State>` и `WorkerBase<State>`.
После этого внутри методов handler'ов или worker'ов можно обратиться к общему состоянию следующим образом
```cpp
struct State {
  yaclib::Mutex<> mtx;
  std::vector<std::uint64_t> some_very_important_info;
};

// ...

// Somewhere inside the handler's or workers's methods 
auto &state = GetState();

{
  auto guard = co_await state.mtx.Guard();
  state.some_very_important_info.push_back(/* ... */);
}
```

### Network

Для взаимодействия с сетью пользователю предоставляется `maelstrom::Network::Session` при каждой обработке (будь то обработка handler'а или worker'а).
Для того, чтобы отправить RPC на другую ноду можно использовать.

```cpp
// Sends a request without waiting for a response
session.SendDetached<DummyHandler>(/*destination=*/"n100", /*body=...*/);

// Sends a request, when the response arrives resolves future
/*yaclib::Future<>*/ session.Send<DummyHandler>(/*destination=*/"n100", /*body=...*/);
/*yaclib::Future<>*/ session.SendAtLeastOnce<DummyHandler>(/*destination=*/"n100", /*body=...*/);

/*yaclib::Future<maelstrom::Response>*/ session.Call<DummyHandler>(/*destination=*/"n100", /*body=...*/);
/*yaclib::Future<maelstrom::Response>*/ session.CallAtLeastOnce<DummyHandler>(/*destination=*/"n100", /*body=...*/);
```