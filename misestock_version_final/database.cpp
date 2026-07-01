#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStringList>
#include <QDateTime>

DataBase::DataBase()
    : nombreConexion("conexion_misestock") {
}

DataBase::~DataBase() {
    cerrarBaseDatos();
}

bool DataBase::abrirBaseDatos(const QString& rutaArchivo) {
    if(rutaArchivo.trimmed().isEmpty()) {
        ultimoError = "La ruta de la base de datos está vacía.";
        return false;
    }

    if(!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        ultimoError = "El driver QSQLITE no está disponible.";
        return false;
    }

    if(QSqlDatabase::contains(nombreConexion)) {
        db = QSqlDatabase::database(nombreConexion);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", nombreConexion);
    }

    if(db.isOpen()) {
        db.close();
    }

    db.setDatabaseName(rutaArchivo);

    if(!db.open()) {
        ultimoError = db.lastError().text();
        return false;
    }

    rutaActual = rutaArchivo;

    QSqlQuery query(db);
    query.exec("PRAGMA foreign_keys = ON");

    return true;
}

void DataBase::cerrarBaseDatos() {
    if(db.isValid() && db.isOpen()) {
        db.close();
    }
}

bool DataBase::estaAbierta() const {
    return db.isValid() && db.isOpen();
}

bool DataBase::validarEstructura() {
    if(!estaAbierta()) {
        ultimoError = "No hay una base de datos abierta.";
        return false;
    }

    QStringList tablas = db.tables();

    if(!tablas.contains("productos")) {
        ultimoError = "La base de datos no contiene la tabla productos.";
        return false;
    }

    if(!tablas.contains("proveedores")) {
        ultimoError = "La base de datos no contiene la tabla proveedores.";
        return false;
    }

    if(!tablas.contains("ventas")) {
        ultimoError = "La base de datos no contiene la tabla ventas.";
        return false;
    }

    if(!tablas.contains("venta_detalle")) {
        ultimoError = "La base de datos no contiene la tabla venta_detalle.";
        return false;
    }

    if(!tablas.contains("movimientos_stock")) {
        ultimoError = "La base de datos no contiene la tabla movimientos_stock.";
        return false;
    }

    QString errorCompras;

    if(!crearTablasCompraSiFaltan(errorCompras)) {
        ultimoError = errorCompras;
        return false;
    }

    return true;
}


bool DataBase::crearTablasCompraSiFaltan(QString& error) {
    if(!estaAbierta()) {
        error = "No hay una base de datos abierta.";
        return false;
    }

    QSqlQuery compras(db);

    if(!compras.exec(
            "CREATE TABLE IF NOT EXISTS compras ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "fecha TEXT NOT NULL, "
            "proveedor_id INTEGER NOT NULL, "
            "estado TEXT NOT NULL DEFAULT 'Pendiente', "
            "total INTEGER NOT NULL DEFAULT 0, "
            "fecha_recepcion TEXT, "
            "FOREIGN KEY(proveedor_id) REFERENCES proveedores(id)"
            ")"
            )) {
        error = compras.lastError().text();
        return false;
    }

    QSqlQuery detalle(db);

    if(!detalle.exec(
            "CREATE TABLE IF NOT EXISTS compra_detalle ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "compra_id INTEGER NOT NULL, "
            "codigo_producto TEXT NOT NULL, "
            "cantidad INTEGER NOT NULL, "
            "costo_unitario INTEGER NOT NULL, "
            "subtotal INTEGER NOT NULL, "
            "FOREIGN KEY(compra_id) REFERENCES compras(id), "
            "FOREIGN KEY(codigo_producto) REFERENCES productos(codigo)"
            ")"
            )) {
        error = detalle.lastError().text();
        return false;
    }

    return true;
}


QString DataBase::getRutaActual() const {
    return rutaActual;
}

QString DataBase::getUltimoError() const {
    return ultimoError;
}

QVector<ProductoDB> DataBase::obtenerProductos(const QString& categoria,
                                               const QString& proveedor,
                                               const QString& filtro) {
    QVector<ProductoDB> productos;

    if(!estaAbierta()) {
        ultimoError = "No hay una base de datos abierta.";
        return productos;
    }

    QString sql =
        "SELECT p.codigo, "
        "p.nombre, "
        "p.categoria, "
        "COALESCE(pr.nombre, 'Sin proveedor') AS proveedor, "
        "p.unidad, "
        "p.stock, "
        "p.stock_seguridad, "
        "p.costo, "
        "p.precio_venta, "
        "COALESCE(pr.lead_time, 0) AS lead_time "
        "FROM productos p "
        "LEFT JOIN proveedores pr ON p.proveedor_id = pr.id "
        "WHERE 1 = 1 ";

    if(categoria != "Todas" && !categoria.trimmed().isEmpty()) {
        sql += "AND p.categoria = :categoria ";
    }

    if(proveedor != "Todos" && !proveedor.trimmed().isEmpty()) {
        sql += "AND pr.nombre = :proveedor ";
    }

    if(!filtro.trimmed().isEmpty()) {
        sql += "AND (p.codigo LIKE :filtro OR p.nombre LIKE :filtro OR p.categoria LIKE :filtro) ";
    }

    sql += "ORDER BY p.nombre";

    QSqlQuery query(db);
    query.prepare(sql);

    if(categoria != "Todas" && !categoria.trimmed().isEmpty()) {
        query.bindValue(":categoria", categoria);
    }

    if(proveedor != "Todos" && !proveedor.trimmed().isEmpty()) {
        query.bindValue(":proveedor", proveedor);
    }

    if(!filtro.trimmed().isEmpty()) {
        query.bindValue(":filtro", "%" + filtro.trimmed() + "%");
    }

    if(!query.exec()) {
        ultimoError = query.lastError().text();
        return productos;
    }

    while(query.next()) {
        ProductoDB producto(
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(4).toString(),
            query.value(5).toInt(),
            query.value(6).toInt(),
            query.value(7).toInt(),
            query.value(8).toInt(),
            query.value(9).toInt()
            );

        productos.push_back(producto);
    }

    return productos;
}

QVector<QString> DataBase::obtenerCategorias() {
    QVector<QString> categorias;

    if(!estaAbierta()) {
        ultimoError = "No hay una base de datos abierta.";
        return categorias;
    }

    QSqlQuery query(db);

    if(!query.exec("SELECT DISTINCT categoria FROM productos ORDER BY categoria")) {
        ultimoError = query.lastError().text();
        return categorias;
    }

    while(query.next()) {
        categorias.push_back(query.value(0).toString());
    }

    return categorias;
}

QVector<QString> DataBase::obtenerProveedores() {
    QVector<QString> proveedores;

    if(!estaAbierta()) {
        ultimoError = "No hay una base de datos abierta.";
        return proveedores;
    }

    QSqlQuery query(db);

    if(!query.exec("SELECT nombre FROM proveedores ORDER BY nombre")) {
        ultimoError = query.lastError().text();
        return proveedores;
    }

    while(query.next()) {
        proveedores.push_back(query.value(0).toString());
    }

    return proveedores;
}

bool DataBase::registrarVenta(const QVector<ItemCarro>& items, QString& error) {
    if(!estaAbierta()) {
        error = "No hay una base de datos abierta.";
        return false;
    }

    if(items.isEmpty()) {
        error = "El carro está vacío.";
        return false;
    }

    if(!db.transaction()) {
        error = db.lastError().text();
        return false;
    }

    //primero se valida que exista stock suficiente en la base real
    for(int i = 0; i < items.size(); i++) {
        QSqlQuery consultaStock(db);
        consultaStock.prepare("SELECT stock FROM productos WHERE codigo = :codigo");
        consultaStock.bindValue(":codigo", items[i].getProducto().getCodigo());

        if(!consultaStock.exec() || !consultaStock.next()) {
            error = "No se pudo consultar el stock del producto " + items[i].getProducto().getCodigo();
            db.rollback();
            return false;
        }

        int stockActual = consultaStock.value(0).toInt();

        if(stockActual < items[i].getCantidad()) {
            error = "Stock insuficiente para el producto " + items[i].getProducto().getNombre();
            db.rollback();
            return false;
        }
    }

    int total = 0;

    for(int i = 0; i < items.size(); i++) {
        total += items[i].calcularSubtotal();
    }

    QString fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    //se guarda la venta general
    QSqlQuery venta(db);
    venta.prepare("INSERT INTO ventas(fecha, total) VALUES(:fecha, :total)");
    venta.bindValue(":fecha", fecha);
    venta.bindValue(":total", total);

    if(!venta.exec()) {
        error = venta.lastError().text();
        db.rollback();
        return false;
    }

    int ventaId = venta.lastInsertId().toInt();

    //se guarda cada producto vendido y se descuenta stock
    for(int i = 0; i < items.size(); i++) {
        ItemCarro item = items[i];

        QSqlQuery detalle(db);
        detalle.prepare(
            "INSERT INTO venta_detalle(venta_id, codigo_producto, cantidad, precio_unitario, subtotal) "
            "VALUES(:venta_id, :codigo_producto, :cantidad, :precio_unitario, :subtotal)"
            );

        detalle.bindValue(":venta_id", ventaId);
        detalle.bindValue(":codigo_producto", item.getProducto().getCodigo());
        detalle.bindValue(":cantidad", item.getCantidad());
        detalle.bindValue(":precio_unitario", item.getPrecioUnitario());
        detalle.bindValue(":subtotal", item.calcularSubtotal());

        if(!detalle.exec()) {
            error = detalle.lastError().text();
            db.rollback();
            return false;
        }

        QSqlQuery actualizaStock(db);
        actualizaStock.prepare("UPDATE productos SET stock = stock - :cantidad WHERE codigo = :codigo");
        actualizaStock.bindValue(":cantidad", item.getCantidad());
        actualizaStock.bindValue(":codigo", item.getProducto().getCodigo());

        if(!actualizaStock.exec()) {
            error = actualizaStock.lastError().text();
            db.rollback();
            return false;
        }

        QSqlQuery movimiento(db);
        movimiento.prepare(
            "INSERT INTO movimientos_stock(fecha, codigo_producto, tipo, cantidad, descripcion) "
            "VALUES(:fecha, :codigo_producto, :tipo, :cantidad, :descripcion)"
            );

        movimiento.bindValue(":fecha", fecha);
        movimiento.bindValue(":codigo_producto", item.getProducto().getCodigo());
        movimiento.bindValue(":tipo", "venta");
        movimiento.bindValue(":cantidad", -item.getCantidad());
        movimiento.bindValue(":descripcion", "venta id " + QString::number(ventaId));

        if(!movimiento.exec()) {
            error = movimiento.lastError().text();
            db.rollback();
            return false;
        }
    }

    if(!db.commit()) {
        error = db.lastError().text();
        return false;
    }

    return true;
}

QVector<HistorialVenta> DataBase::obtenerHistorialVentas(const QString& periodo) {
    QVector<HistorialVenta> historial;

    if(!estaAbierta()) {
        ultimoError = "No hay una base de datos abierta.";
        return historial;
    }

    QString sql =
        "SELECT v.fecha, "
        "'Venta' AS tipo, "
        "d.codigo_producto, "
        "p.nombre, "
        "d.cantidad, "
        "d.precio_unitario, "
        "d.subtotal, "
        "'venta id ' || v.id AS descripcion "
        "FROM ventas v "
        "INNER JOIN venta_detalle d ON v.id = d.venta_id "
        "INNER JOIN productos p ON d.codigo_producto = p.codigo "
        "WHERE 1 = 1 ";

    if(periodo == "Hoy") {
        sql += "AND date(v.fecha) = date('now', 'localtime') ";
    } else if(periodo == "Últimos 7 días") {
        sql += "AND datetime(v.fecha) >= datetime('now', 'localtime', '-7 days') ";
    } else if(periodo == "Este mes") {
        sql += "AND strftime('%Y-%m', v.fecha) = strftime('%Y-%m', 'now', 'localtime') ";
    }

    sql += "ORDER BY v.fecha DESC, v.id DESC";

    QSqlQuery query(db);

    if(!query.exec(sql)) {
        ultimoError = query.lastError().text();
        return historial;
    }

    while(query.next()) {
        HistorialVenta venta(
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(4).toInt(),
            query.value(5).toInt(),
            query.value(6).toInt(),
            query.value(7).toString()
            );

        historial.push_back(venta);
    }

    return historial;
}

QVector<HistorialVenta> DataBase::obtenerHistorialCompras(const QString& periodo) {
    QVector<HistorialVenta> historial;

    if(!estaAbierta()) {
        ultimoError = "No hay una base de datos abierta.";
        return historial;
    }

    QString sql =
        "SELECT COALESCE(c.fecha_recepcion, c.fecha) AS fecha, "
        "'Compra' AS tipo, "
        "d.codigo_producto, "
        "p.nombre, "
        "d.cantidad, "
        "d.costo_unitario, "
        "d.subtotal, "
        "'orden compra id ' || c.id || ' - ' || c.estado AS descripcion "
        "FROM compras c "
        "INNER JOIN compra_detalle d ON c.id = d.compra_id "
        "INNER JOIN productos p ON d.codigo_producto = p.codigo "
        "WHERE c.estado = 'Recibida' ";

    if(periodo == "Hoy") {
        sql += "AND date(COALESCE(c.fecha_recepcion, c.fecha)) = date('now', 'localtime') ";
    } else if(periodo == "Últimos 7 días") {
        sql += "AND datetime(COALESCE(c.fecha_recepcion, c.fecha)) >= datetime('now', 'localtime', '-7 days') ";
    } else if(periodo == "Este mes") {
        sql += "AND strftime('%Y-%m', COALESCE(c.fecha_recepcion, c.fecha)) = strftime('%Y-%m', 'now', 'localtime') ";
    }

    sql += "ORDER BY fecha DESC, c.id DESC";

    QSqlQuery query(db);

    if(!query.exec(sql)) {
        ultimoError = query.lastError().text();
        return historial;
    }

    while(query.next()) {
        HistorialVenta compra(
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(4).toInt(),
            query.value(5).toInt(),
            query.value(6).toInt(),
            query.value(7).toString()
            );

        historial.push_back(compra);
    }

    return historial;
}

double DataBase::calcularDemandaPromedioMovil(const QString& codigoProducto, int dias) {
    //este metodo calcula la demanda promedio diaria de un producto
    if(!estaAbierta()) {
        ultimoError = "No hay una base de datos abierta.";
        return 0.0;
    }

    if(dias <= 0) {
        return 0.0;
    }

    int diasHaciaAtras = dias - 1;
    QString modificadorFecha = "-" + QString::number(diasHaciaAtras) + " days";

    QSqlQuery query(db);

    query.prepare(
        "SELECT COALESCE(SUM(d.cantidad), 0) "
        "FROM ventas v "
        "INNER JOIN venta_detalle d ON v.id = d.venta_id "
        "WHERE d.codigo_producto = :codigo "
        "AND date(v.fecha) >= date('now', 'localtime', :modificador)"
        );

    query.bindValue(":codigo", codigoProducto);
    query.bindValue(":modificador", modificadorFecha);

    if(!query.exec()) {
        ultimoError = query.lastError().text();
        return 0.0;
    }

    if(!query.next()) {
        return 0.0;
    }

    int cantidadVendida = query.value(0).toInt();

    return static_cast<double>(cantidadVendida) / dias;
}


bool DataBase::registrarOrdenCompra(const QVector<ItemCompra>& items, QString& error) {
    if(!estaAbierta()) {
        error = "No hay una base de datos abierta.";
        return false;
    }

    if(items.isEmpty()) {
        error = "La orden está vacía.";
        return false;
    }

    QString proveedor = items[0].getProducto().getProveedor();

    if(proveedor.trimmed().isEmpty() || proveedor == "Todos" || proveedor == "Sin proveedor") {
        error = "La orden debe tener un proveedor válido.";
        return false;
    }

    for(int i = 0; i < items.size(); i++) {
        if(items[i].getProducto().getProveedor() != proveedor) {
            error = "Todos los productos de la orden deben pertenecer al mismo proveedor.";
            return false;
        }
    }

    QSqlQuery consultaProveedor(db);
    consultaProveedor.prepare("SELECT id FROM proveedores WHERE nombre = :nombre");
    consultaProveedor.bindValue(":nombre", proveedor);

    if(!consultaProveedor.exec() || !consultaProveedor.next()) {
        error = "No se pudo encontrar el proveedor " + proveedor;
        return false;
    }

    int proveedorId = consultaProveedor.value(0).toInt();

    if(!db.transaction()) {
        error = db.lastError().text();
        return false;
    }

    int total = 0;

    for(int i = 0; i < items.size(); i++) {
        total += items[i].calcularSubtotal();
    }

    QString fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QSqlQuery compra(db);
    compra.prepare(
        "INSERT INTO compras(fecha, proveedor_id, estado, total) "
        "VALUES(:fecha, :proveedor_id, :estado, :total)"
        );

    compra.bindValue(":fecha", fecha);
    compra.bindValue(":proveedor_id", proveedorId);
    compra.bindValue(":estado", "Pendiente");
    compra.bindValue(":total", total);

    if(!compra.exec()) {
        error = compra.lastError().text();
        db.rollback();
        return false;
    }

    int compraId = compra.lastInsertId().toInt();

    for(int i = 0; i < items.size(); i++) {
        ItemCompra item = items[i];

        QSqlQuery detalle(db);
        detalle.prepare(
            "INSERT INTO compra_detalle(compra_id, codigo_producto, cantidad, costo_unitario, subtotal) "
            "VALUES(:compra_id, :codigo_producto, :cantidad, :costo_unitario, :subtotal)"
            );

        detalle.bindValue(":compra_id", compraId);
        detalle.bindValue(":codigo_producto", item.getProducto().getCodigo());
        detalle.bindValue(":cantidad", item.getCantidad());
        detalle.bindValue(":costo_unitario", item.getCostoUnitario());
        detalle.bindValue(":subtotal", item.calcularSubtotal());

        if(!detalle.exec()) {
            error = detalle.lastError().text();
            db.rollback();
            return false;
        }
    }

    if(!db.commit()) {
        error = db.lastError().text();
        return false;
    }

    return true;
}

QVector<OrdenCompra> DataBase::obtenerOrdenesCompra() {
    QVector<OrdenCompra> ordenes;

    if(!estaAbierta()) {
        ultimoError = "No hay una base de datos abierta.";
        return ordenes;
    }

    QSqlQuery query(db);

    if(!query.exec(
            "SELECT c.id, c.fecha, pr.nombre, c.estado, c.total "
            "FROM compras c "
            "INNER JOIN proveedores pr ON c.proveedor_id = pr.id "
            "ORDER BY c.fecha DESC, c.id DESC"
            )) {
        ultimoError = query.lastError().text();
        return ordenes;
    }

    while(query.next()) {
        OrdenCompra orden(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toString(),
            query.value(4).toInt()
            );

        ordenes.push_back(orden);
    }

    return ordenes;
}

bool DataBase::marcarOrdenCompraRecibida(int idCompra, QString& error) {
    if(!estaAbierta()) {
        error = "No hay una base de datos abierta.";
        return false;
    }

    QSqlQuery consultaOrden(db);
    consultaOrden.prepare("SELECT estado FROM compras WHERE id = :id");
    consultaOrden.bindValue(":id", idCompra);

    if(!consultaOrden.exec() || !consultaOrden.next()) {
        error = "No se pudo encontrar la orden seleccionada.";
        return false;
    }

    QString estado = consultaOrden.value(0).toString();

    if(estado == "Recibida") {
        error = "La orden ya fue recibida.";
        return false;
    }

    if(estado != "Pendiente") {
        error = "Solo se pueden recibir órdenes pendientes.";
        return false;
    }

    if(!db.transaction()) {
        error = db.lastError().text();
        return false;
    }

    struct DetalleCompraLocal {
        QString codigo;
        int cantidad;
    };

    QVector<DetalleCompraLocal> detalles;

    QSqlQuery consultaDetalle(db);
    consultaDetalle.prepare(
        "SELECT codigo_producto, cantidad "
        "FROM compra_detalle "
        "WHERE compra_id = :compra_id"
        );

    consultaDetalle.bindValue(":compra_id", idCompra);

    if(!consultaDetalle.exec()) {
        error = consultaDetalle.lastError().text();
        db.rollback();
        return false;
    }

    while(consultaDetalle.next()) {
        DetalleCompraLocal detalle;
        detalle.codigo = consultaDetalle.value(0).toString();
        detalle.cantidad = consultaDetalle.value(1).toInt();
        detalles.push_back(detalle);
    }

    if(detalles.isEmpty()) {
        error = "La orden no tiene productos.";
        db.rollback();
        return false;
    }

    QString fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    for(int i = 0; i < detalles.size(); i++) {
        QSqlQuery actualizaStock(db);
        actualizaStock.prepare(
            "UPDATE productos "
            "SET stock = stock + :cantidad "
            "WHERE codigo = :codigo"
            );

        actualizaStock.bindValue(":cantidad", detalles[i].cantidad);
        actualizaStock.bindValue(":codigo", detalles[i].codigo);

        if(!actualizaStock.exec()) {
            error = actualizaStock.lastError().text();
            db.rollback();
            return false;
        }

        QSqlQuery movimiento(db);
        movimiento.prepare(
            "INSERT INTO movimientos_stock(fecha, codigo_producto, tipo, cantidad, descripcion) "
            "VALUES(:fecha, :codigo_producto, :tipo, :cantidad, :descripcion)"
            );

        movimiento.bindValue(":fecha", fecha);
        movimiento.bindValue(":codigo_producto", detalles[i].codigo);
        movimiento.bindValue(":tipo", "compra");
        movimiento.bindValue(":cantidad", detalles[i].cantidad);
        movimiento.bindValue(":descripcion", "orden compra id " + QString::number(idCompra));

        if(!movimiento.exec()) {
            error = movimiento.lastError().text();
            db.rollback();
            return false;
        }
    }

    QSqlQuery actualizaOrden(db);
    actualizaOrden.prepare(
        "UPDATE compras "
        "SET estado = :estado, fecha_recepcion = :fecha_recepcion "
        "WHERE id = :id"
        );

    actualizaOrden.bindValue(":estado", "Recibida");
    actualizaOrden.bindValue(":fecha_recepcion", fecha);
    actualizaOrden.bindValue(":id", idCompra);

    if(!actualizaOrden.exec()) {
        error = actualizaOrden.lastError().text();
        db.rollback();
        return false;
    }

    if(!db.commit()) {
        error = db.lastError().text();
        return false;
    }

    return true;
}

bool DataBase::cancelarOrdenCompra(int idCompra, QString& error) {
    if(!estaAbierta()) {
        error = "No hay una base de datos abierta.";
        return false;
    }

    QSqlQuery consultaOrden(db);
    consultaOrden.prepare("SELECT estado FROM compras WHERE id = :id");
    consultaOrden.bindValue(":id", idCompra);

    if(!consultaOrden.exec() || !consultaOrden.next()) {
        error = "No se pudo encontrar la orden seleccionada.";
        return false;
    }

    QString estado = consultaOrden.value(0).toString();

    if(estado == "Recibida") {
        error = "No se puede cancelar una orden que ya fue recibida.";
        return false;
    }

    if(estado == "Cancelada") {
        error = "La orden ya está cancelada.";
        return false;
    }

    QSqlQuery actualizaOrden(db);
    actualizaOrden.prepare(
        "UPDATE compras "
        "SET estado = :estado "
        "WHERE id = :id"
        );

    actualizaOrden.bindValue(":estado", "Cancelada");
    actualizaOrden.bindValue(":id", idCompra);

    if(!actualizaOrden.exec()) {
        error = actualizaOrden.lastError().text();
        return false;
    }

    return true;
}
