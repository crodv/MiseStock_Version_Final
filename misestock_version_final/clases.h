#ifndef CLASES_H
#define CLASES_H

#include <QString>

//////////// PRODUCTO DESDE BASE DE DATOS //////////////

class ProductoDB {
private:
    QString codigo;
    QString nombre;
    QString categoria;
    QString proveedor;
    QString unidad;
    int stock;
    int stockSeguridad;
    int costo;
    int precioVenta;
    int leadTime;

public:
    ProductoDB();

    ProductoDB(QString codigo, QString nombre, QString categoria, QString proveedor,
               QString unidad, int stock, int stockSeguridad, int costo, int precioVenta, int leadTime = 0);

    QString getCodigo() const;
    QString getNombre() const;
    QString getCategoria() const;
    QString getProveedor() const;
    QString getUnidad() const;
    int getStock() const;
    int getStockSeguridad() const;
    int getCosto() const;
    int getPrecioVenta() const;
    int getLeadTime() const;
};


//////////// PRODUCTO PARA VENTA //////////////

class ProductoVenta {
private:
    QString codigo;
    QString nombre;
    QString categoria;
    QString proveedor;
    QString unidad;
    int stock;
    int precioVenta;

public:
    ProductoVenta();

    ProductoVenta(QString codigo, QString nombre, QString categoria, QString proveedor,
                  QString unidad, int stock, int precioVenta);

    QString getCodigo() const;
    QString getNombre() const;
    QString getCategoria() const;
    QString getProveedor() const;
    QString getUnidad() const;
    int getStock() const;
    int getPrecioVenta() const;
};


//////////// ITEM DEL CARRO //////////////

class ItemCarro {
private:
    ProductoVenta producto;
    int cantidad;

public:
    ItemCarro();

    ItemCarro(ProductoVenta producto, int cantidad);

    ProductoVenta getProducto() const;
    int getCantidad() const;
    int getPrecioUnitario() const;
    int calcularSubtotal() const;

    void aumentarCantidad(int cantidadExtra);
};


//////////// HISTORIAL DE VENTA //////////////

class HistorialVenta {
private:
    QString fecha;
    QString tipo;
    QString codigo;
    QString producto;
    int cantidad;
    int precioUnitario;
    int subtotal;
    QString descripcion;

public:
    HistorialVenta();

    HistorialVenta(QString fecha, QString tipo, QString codigo, QString producto,
                   int cantidad, int precioUnitario, int subtotal, QString descripcion);

    QString getFecha() const;
    QString getTipo() const;
    QString getCodigo() const;
    QString getProducto() const;
    int getCantidad() const;
    int getPrecioUnitario() const;
    int getSubtotal() const;
    QString getDescripcion() const;
};


//////////// ITEM DE COMPRA //////////////

class ItemCompra {
private:
    ProductoDB producto;
    int cantidad;

public:
    ItemCompra();
    ItemCompra(ProductoDB producto, int cantidad);

    ProductoDB getProducto() const;
    int getCantidad() const;
    int getCostoUnitario() const;
    int calcularSubtotal() const;

    void aumentarCantidad(int cantidadExtra);
};


//////////// ORDEN DE COMPRA //////////////

class OrdenCompra {
private:
    int id;
    QString fecha;
    QString proveedor;
    QString estado;
    int total;

public:
    OrdenCompra();
    OrdenCompra(int id, QString fecha, QString proveedor, QString estado, int total);

    int getId() const;
    QString getFecha() const;
    QString getProveedor() const;
    QString getEstado() const;
    int getTotal() const;
};
#endif
