# RPC Setup 

## Option I 

Properties / Build Events / Pre Build Event 
속성 / 빌드 이벤트 / 사전 빌드 이벤트 
```
cd "$(ProjectDir)"
python proxy_gen.py
```

장점 : 설정 1줄 
단점 : 매 빌드마다 실행 

