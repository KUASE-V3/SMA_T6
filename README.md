# 개발 환경 설정 및 CI/CD 사용법

이 문서는 **Docker Compose, CMake, GoogleTest, Gcovr, cppcheck 및 SonarCloud**를 이용하여 **일관된 개발 환경을 구축**하고 **코드 품질을 자동 검증하는 방법**을 설명합니다.

---

## 🛠 개발 환경 설정

### Docker 명령어
Docker Compose를 이용하여 모든 개발자가 동일한 환경을 사용할 수 있도록 설정합니다.

| 명령어 | 설명 |
|--------|-----------------------------|
| `docker-compose up -d` | 개발환경 구축 및 시작 |
| `docker-compose stop` | 기존 컨테이너를 멈춤 |
| `docker-compose start` | 기존 컨테이너를 다시 시작 |
| `docker-compose down` | 개발 컨테이너를 제거 |

📌 모든 개발자가 동일한 컨테이너 이름을 사용하기 위해 `container_name`을 설정하세요.

---

## 💻 컴파일 및 빌드

### 기본 빌드 명령어
```sh
cd build
cmake ..
cmake --build .
```
CMake를 이용하여 프로젝트를 빌드합니다.

---

## 🧪 테스트 실행 (GoogleTest)

### GoogleTest 빌드 및 실행
```sh
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build . --target googletest
```
✅ **테스트 활성화(`BUILD_TESTS=ON`)** 후 GoogleTest를 빌드하고 실행할 수 있습니다.

---

## 📊 테스트 커버리지 확인 (Gcovr)

### Gcovr 실행 절차
```sh
cd build
cmake .. -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
cmake --build . --target googletest
ctest -V  # 테스트 실행 (한 번 이상 필요함)
cmake --build . --target coverage
```
📌 실행 후 `build/coverage/coverage.html` 파일에서 커버리지 결과를 확인할 수 있습니다.

---

## 🧐 정적 분석 (Cppcheck)

### Cppcheck 실행 명령어
```sh
cd build
cmake ..
cmake --build . --target cppcheck
```
📌 **Cppcheck를 통해 코드 품질을 검사**하고, 잠재적인 버그를 찾아낼 수 있습니다.

---

## 🔍 SonarCloud 사용법 (자동 코드 분석)

### SonarCloud 분석 실행 방법
SonarCloud는 **GitHub Actions와 연동**되어, **코드를 push 하면 자동으로 분석이 수행**됩니다.  
따라서 별도의 실행 명령 없이 **GitHub에 코드 변경 사항을 push**하면 자동으로 코드 품질 및 보안 검사가 진행됩니다.
