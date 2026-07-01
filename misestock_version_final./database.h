#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QVector>
#include <QSqlDatabase>
#include "clases.h"

//clase encargada de abrir sqlite y obtener datos
class DataBase {
private:
    QSqlDatabase db;
    QString rutaActual;
    QString ultimoError;
    QString nombreConexion;
    bool crearTablasCompraSiFaltan(QString& error);

public:
    DataBase();
    ~DataBase();

    bool abrirBaseDatos(const QString& rutaArchivo);
    void cerrarBaseDatos();

    bool estaAbierta() const;
    bool validarEstructura();

    QString getRutaActual() const;
    QString getUltimoError() const;

    QVector<ProductoDB> obtenerProductos(const QString& categoria,
                                         const QString& proveedor,
                                         const QString& filtro);

    QVector<QString> obtenerCategorias();
    QVector<QString> obtenerProveedores();

    bool registrarVenta(const QVector<ItemCarro>& items, QString& error);

    QVector<HistorialVenta> obtenerHistorialVentas(const QString& periodo);
    QVector<HistorialVenta> obtenerHistorialCompras(const QString& periodo);

    double calcularDemandaPromedioMovil(const QString& codigoProducto, int dias);

    //de la pestaña compras
    bool registrarOrdenCompra(const QVector<ItemCompra>& items, QString& error);
    QVector<OrdenCompra> obtenerOrdenesCompra();
    bool marcarOrdenCompraRecibida(int idCompra, QString& error);
    bool cancelarOrdenCompra(int idCompra, QString& error);
};

#endif
