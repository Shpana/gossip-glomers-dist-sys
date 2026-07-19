# Комментарии к решению

- Каждая нода пишет в свою ячейку kv-storage'а, чтобы минимизировать contention (в частности при кратном увеличении количества нод).
- `add`-запросы в CAS-цикле пытаются увеличить счетчик на полученное `delta`.
- `read`-запросы читают все значения из соответсвующих нодам ячеек. Причем так как kv-storage только sequential consistent, то перед каждым чтением делается фиктивный CAS-запрос, чтобы установить 'progam-order' для последующих чтений.

## Оптимизации

### Кэширование `read`-запросов
Так как наш сервис должен быть eventualy consistent, причем разрешается задерживать обновление счетчика на несколько секунд, то можно кэшировать `read`-запросы.

При `cache_invalidation_timeout=1s` latency `read`-запросов становится лучше
<p align="center">
  <img src="/tasks/gcounter/assets/latency-no-cache.png" width="45%"/>
  <img src="/tasks/gcounter/assets/latency-cache.png" width="45%"/> 
</p>
