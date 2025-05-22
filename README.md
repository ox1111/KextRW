

# KextRW

**KextRW**는 macOS의 XNU 커널을 대상으로 한 **보안 및 취약점 연구에 유용한 기능들을 제공하는 커널 확장(Kernel Extension)** 입니다.

## ✨ 주요 기능

이 커널 확장은 다음과 같은 핵심 기능을 제공합니다:

- 🧠 **가상 커널 메모리 읽기/쓰기** (Virtual kernel read/write)  
- 🧱 **물리 메모리 읽기/쓰기** (Physical memory read/write)  
- 🧩 **커널 베이스 및 슬라이드 값 획득** (Kernel base and KASLR slide)  
- 🗃 **커널 메모리 할당 및 해제** (Kernel memory allocation/freeing)  
- 📞 **커널 함수 호출 primitive 제공** (Kernel call primitive)  
- 🔁 **주소 변환 기능** (Virtual/physical address translation)

> 🔧 이 프로젝트는 [IOKernelRW](https://github.com/ox1111/IOKernelRW)를 기반으로 만들어졌으며, **설치 방법도 동일**합니다.

## 🔐 사용 권한

이 커널 확장을 사용하는 모든 바이너리는 아래 **entitlement**를 반드시 가져야 합니다:

```xml
com.apple.security.alfie.kext-rw
```

## 🧪 테스트 프로그램

`tests/` 디렉토리에 테스트용 C 코드(`kextrw_test.c`)가 포함되어 있습니다.  
단, 이 코드에 사용된 **offset 및 주소는 작성자의 시스템에 맞춰져 있으므로**,  
다른 시스템에서는 `kextrw_test.c` 내 상수를 수정해야 정상 작동합니다.

## 📍 커널캐시 경로 확인

테스트를 위해 **커널 오프셋**을 확인하고 수정할 필요가 있습니다.  
macOS에서 아래 명령어를 실행하여 `kernelcache` 경로를 확인하세요:

```bash
kmutil inspect
```

→ 출력 상단에 경로가 표시됩니다. 해당 경로를 바탕으로 분석하여 주소를 갱신할 수 있습니다.

## 🏗 빌드 방법

전체 프로젝트를 빌드하려면 아래 명령어를 실행합니다:

```bash
make all
```

빌드 결과물은 `build/` 디렉토리에 저장되며:

- `libkextrw.a` (정적 라이브러리)
- `kextrw.h` (헤더 파일)

위 파일들을 사용하여, 커널 primitive 기능을 활용하는 프로젝트를 개발할 수 있습니다.

## 💡 예시 코드

예제는 `tests/kextrw_test.c` 파일에 있습니다.  
이 코드는 다음과 같은 기능을 시연합니다:

- 커널 읽기/쓰기
- 주소 변환
- 커널 함수 호출 등

## ⚠️ 호환성

- 현재 **macOS 15.2**에서만 테스트되었습니다.
- 다른 macOS 버전에서는 offset 수정이 필요할 수 있습니다.

## 📚 참고

- 기반 프로젝트: [IOKernelRW](https://github.com/ox1111/IOKernelRW)
- 보안 연구 및 XNU 커널 취약점 분석에 사용됨
- macOS에서 KEXT 개발 시 entitlement 필요

## 🧑‍💻 라이선스

> 본 프로젝트의 라이선스는 IOKernelRW의 라이선스와 동일합니다. (MIT 또는 BSD 유사)






# KextRW

A macOS kernel extension offering several features useful for security/vulnerability research against XNU.

The features provided by this kernel extension include:
* Virtual kernel read/write
* Physical read/write
* Getting the kernel base and slide
* Kernel memory allocation and freeing
* Kernel call primitive
* Address translation

The codebase is originally based on [IOKernelRW](https://github.com/ox1111/IOKernelRW), where you can find installation instructions, as they will be the same for this project. Any binary that wishes to create a userclient for this kernel extension must possess the `com.apple.security.alfie.kext-rw` entitlement. An easy-to-use test program can be found in the `tests/` folder, but the offsets and addresses are specific to my machine.

You can find the path to your kernelcache by running `kmutil inspect` - it will be printed at the top. This will be necessary if you would like to update the offsets and addresses in `kextrw_test.c`. I have not tested this on anything other than macOS 15.2.

Building the project using `make all` will output a static `libkextrw` library and a header file in the `build/` directory, which you can then use to build projects on top of the primitives provided by the kernel extension. The `kextrw_test.c` file in the `tests/` directory offers an example of this use case.
