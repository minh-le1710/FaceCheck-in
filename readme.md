flowchart TD
  subgraph UPLOAD
    A1[FE Image + shopID] -->|HTTP| B1[BE Postgre]
    B1 -->|HTTP: shopID, employeeID, image| C1[BE Deepface]
    C1 -->|Upsert vector| D1[PineconeDB]
  end

  subgraph QUERY
    A2[ShopBE Camera: image + shopID] -->|HTTP| B2[BE Postgre]
    B2 -->|HTTP: shopID, image| C2[BE Deepface]
    C2 -->|Query vector| D2[PineconeDB]
    D2 -->|vectorID| E2[BE Postgre]
    E2 -->|HTTP: success/failure| F2[Log]
    C2 -->|HTTP: success/failure| F2
    C2 -->|MQTT: success/failure| G2[ESP8266 Board]
  end
