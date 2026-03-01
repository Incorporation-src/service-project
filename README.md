# Краткое описание проекта
Учебный проект, демонстрирующий микросервисную архитектуру на языке C++. Приложение состоит из двух независимых микросервисов, взаимодействующих между собой по HTTP и использующих отдельные базы данных.

User Service (порт 8081) — отвечает за управление пользователями:
- Создание, просмотр, обновление и удаление пользователей
- Поиск пользователей по email
- Хранение данных в SQLite (data/users.db)

Order Service (порт 8082) — отвечает за управление заказами:
- Создание, просмотр, обновление статуса и удаление заказов
- Получение заказов конкретного пользователя
- Хранение данных в SQLite (data/orders.db)

# Используемые технологии
- C++17	        Язык программирования
- Crow	        HTTP-фреймворк для создания REST API
- SQLite3	    Лёгкая встраиваемая база данных
- libcurl	    HTTP-клиент для межсервисного взаимодействия
- CMake	        Система сборки
- REST API	    Архитектурный стиль взаимодействия
- JSON	        Формат обмена данными

# Команды для тестов

## Сборка проекта
### Собрать все сервисы
./cmake_build.sh


## Запуск сервисов
### Терминал 1 - User Service (порт 8081)
cd ~/service-project/build
./user_service
### Терминал 2 - Order Service (порт 8082)
cd ~/service-project/build
./order_service


## Проверка работоспособности
### Проверка статуса сервисов
curl http://localhost:8081/status
curl http://localhost:8082/status

Ожидаемый ответ:
{"status":"running","service":"user-service","version":1}
{"status":"running","service":"order-service","version":1}


## Тестирование User Service
### Создать пользователя
curl -X POST http://localhost:8081/users \
  -H "Content-Type: application/json" \
  -d '{"name":"Иван Петров","email":"ivan@example.com"}'
### Получить всех пользователей
curl http://localhost:8081/users
### Получить пользователя по ID
curl http://localhost:8081/users/1
### Найти пользователя по email
curl "http://localhost:8081/users/email?address=ivan@example.com"
### Обновить данные пользователя
curl -X PUT http://localhost:8081/users/1 \
  -H "Content-Type: application/json" \
  -d '{"name":"Иван Иванов","email":"ivan.new@example.com"}'
### Удалить пользователя
curl -X DELETE http://localhost:8081/users/1


## Тестирование Order Service
### Создать заказ (для пользователя с ID=1)
curl -X POST http://localhost:8082/orders \
  -H "Content-Type: application/json" \
  -d '{"user_id":1,"title":"Ноутбук","amount":75000.50}'
curl -X POST http://localhost:8082/orders \
  -H "Content-Type: application/json" \
  -d '{"user_id":1,"title":"Мышь","amount":1500.00}'
### Получить все заказы
curl http://localhost:8082/orders
### Получить заказ по ID
curl http://localhost:8082/orders/1
### Получить заказы конкретного пользователя
curl http://localhost:8082/orders/user/1
### Обновить статус заказа
curl -X PUT http://localhost:8082/orders/1/status \
  -H "Content-Type: application/json" \
  -d '{"status":"shipped"}'
### Удалить заказ
curl -X DELETE http://localhost:8082/orders/1


## Взаимодействие сервисов
### Создать пользователя (User Service)
curl -X POST http://localhost:8081/users \
  -H "Content-Type: application/json" \
  -d '{"name":"Иван Петров","email":"ivan.petrov@example.com"}'
### Создать несколько заказов для него (Order Service)
curl -X POST http://localhost:8082/orders \
  -H "Content-Type: application/json" \
  -d '{"user_id":1,"title":"Ноутбук","amount":75000.50}'
curl -X POST http://localhost:8082/orders \
  -H "Content-Type: application/json" \
  -d '{"user_id":1,"title":"Мышь","amount":1500.00}'
curl -X POST http://localhost:8082/orders \
  -H "Content-Type: application/json" \
  -d '{"user_id":1,"title":"Клавиатура","amount":3500.00}'
### Получить пользователя и его заказы
curl http://localhost:8081/users/1/with-orders | json_pp
