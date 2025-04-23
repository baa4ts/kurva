# Comandos SC para administrar un servicio en Windows

## 1. Crear el servicio
```cmd
sc create "Windows Runtime" binPath= "C:\Users\baa4t\OneDrive\Documentos\kurva\build\32\kurva.exe"
```

## 2. Iniciar el servicio
```cmd
sc start "Windows Runtime"
```

## 3. Verificar si el servicio está corriendo
```cmd
sc query "Windows Runtime"
```

## 4. Detener el servicio
```cmd
sc stop "Windows Runtime"
```

## 5. Eliminar el servicio
```cmd
sc delete "Windows Runtime"
```