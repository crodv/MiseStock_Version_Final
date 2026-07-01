#include "clases.h"


//////////// PRODUCTO DESDE BASE DE DATOS //////////////

ProductoDB::ProductoDB() : stock(0), stockSeguridad(0), costo(0), precioVenta(0), leadTime(0) { }

ProductoDB::ProductoDB(QString codigo, QString nombre, QString categoria, QString proveedor,
                       QString unidad, int stock, int stockSeguridad, int costo,
                       int precioVenta, int leadTime)
    : codigo(codigo), nombre(nombre), categoria(categoria), proveedor(proveedor), unidad(unidad),
    stock(stock), stockSeguridad(stockSeguridad), costo(costo),
    precioVenta(precioVenta), leadTime(leadTime) {
}

QString ProductoDB::getCodigo() const {
    return codigo;
}

QString ProductoDB::getNombre() const {
    return nombre;
}

QString ProductoDB::getCategoria() const {
    return categoria;
}

QString ProductoDB::getProveedor() const {
    return proveedor;
}

QString ProductoDB::getUnidad() const {
    return unidad;
}

int ProductoDB::getStock() const {
    return stock;
}

int ProductoDB::getStockSeguridad() const {
    return stockSeguridad;
}

int ProductoDB::getCosto() const {
    return costo;
}

int ProductoDB::getPrecioVenta() const {
    return precioVenta;
}

int ProductoDB::getLeadTime() const {
    return leadTime;
}

//////////// PRODUCTO PARA VENTA //////////////

ProductoVenta::ProductoVenta()
    : stock(0), precioVenta(0) {
}

ProductoVenta::ProductoVenta(QString codigo, QString nombre, QString categoria, QString proveedor,
                             QString unidad, int stock, int precioVenta)
    : codigo(codigo), nombre(nombre), categoria(categoria), proveedor(proveedor),
    unidad(unidad), stock(stock), precioVenta(precioVenta) {
}

QString ProductoVenta::getCodigo() const {
    return codigo;
}

QString ProductoVenta::getNombre() const {
    return nombre;
}

QString ProductoVenta::getCategoria() const {
    return categoria;
}

QString ProductoVenta::getProveedor() const {
    return proveedor;
}

QString ProductoVenta::getUnidad() const {
    return unidad;
}

int ProductoVenta::getStock() const {
    return stock;
}

int ProductoVenta::getPrecioVenta() const {
    return precioVenta;
}


//////////// ITEM DEL CARRO //////////////

ItemCarro::ItemCarro()
    : cantidad(0) {
}

ItemCarro::ItemCarro(ProductoVenta producto, int cantidad)
    : producto(producto), cantidad(cantidad) {
}

ProductoVenta ItemCarro::getProducto() const {
    return producto;
}

int ItemCarro::getCantidad() const {
    return cantidad;
}

int ItemCarro::getPrecioUnitario() const {
    return producto.getPrecioVenta();
}

int ItemCarro::calcularSubtotal() const {
    return cantidad * producto.getPrecioVenta();
}

void ItemCarro::aumentarCantidad(int cantidadExtra) {
    cantidad += cantidadExtra;
}


//////////// HISTORIAL DE VENTA //////////////

HistorialVenta::HistorialVenta()
    : cantidad(0), precioUnitario(0), subtotal(0) {
}

HistorialVenta::HistorialVenta(QString fecha, QString tipo, QString codigo, QString producto,
                               int cantidad, int precioUnitario, int subtotal, QString descripcion)
    : fecha(fecha), tipo(tipo), codigo(codigo), producto(producto),
    cantidad(cantidad), precioUnitario(precioUnitario), subtotal(subtotal),
    descripcion(descripcion) {
}

QString HistorialVenta::getFecha() const {
    return fecha;
}

QString HistorialVenta::getTipo() const {
    return tipo;
}

QString HistorialVenta::getCodigo() const {
    return codigo;
}

QString HistorialVenta::getProducto() const {
    return producto;
}

int HistorialVenta::getCantidad() const {
    return cantidad;
}

int HistorialVenta::getPrecioUnitario() const {
    return precioUnitario;
}

int HistorialVenta::getSubtotal() const {
    return subtotal;
}

QString HistorialVenta::getDescripcion() const {
    return descripcion;
}


//////////// ITEM DE COMPRA //////////////

ItemCompra::ItemCompra() : cantidad(0) {
}

ItemCompra::ItemCompra(ProductoDB producto, int cantidad)
    : producto(producto), cantidad(cantidad) {
}

ProductoDB ItemCompra::getProducto() const {
    return producto;
}

int ItemCompra::getCantidad() const {
    return cantidad;
}

int ItemCompra::getCostoUnitario() const {
    return producto.getCosto();
}

int ItemCompra::calcularSubtotal() const {
    return cantidad * producto.getCosto();
}

void ItemCompra::aumentarCantidad(int cantidadExtra) {
    cantidad += cantidadExtra;
}


//////////// ORDEN DE COMPRA //////////////

OrdenCompra::OrdenCompra() : id(0), total(0) {
}

OrdenCompra::OrdenCompra(int id, QString fecha, QString proveedor, QString estado, int total)
    : id(id), fecha(fecha), proveedor(proveedor), estado(estado), total(total) {
}

int OrdenCompra::getId() const {
    return id;
}

QString OrdenCompra::getFecha() const {
    return fecha;
}

QString OrdenCompra::getProveedor() const {
    return proveedor;
}

QString OrdenCompra::getEstado() const {
    return estado;
}

int OrdenCompra::getTotal() const {
    return total;
}
