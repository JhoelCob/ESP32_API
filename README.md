%% Diagrama ESP32 + API
```mermaid
graph TD
    A[ESP32 Conexión a una red] --> |No|B[Busqueda de redes]

    A --> |Si|G[API]
    G --> |Protocolo HTTP|H[Envio de un formato JSON al servidor]
    H --> I[Respuesta del servidor]
    I --> |Si|J[Procesamiento de datos]
    I --> |No|K[Alerta fallo]
    B --> C[Seleccion de red]
    C --> D[Introduccion de contraseña]
    D --> E[Acceso a red]
    E --> F[Almacenamiento de contraseña]
    F --> A
