#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QStringList>
#include <QtMath>


//////////// CONFIGURACION DE COMPRA //////////////

void MainWindow::configurarCompra() {
    //configura la tabla de productos ofrecidos por proveedores
    ui->tableProductosCompra->setColumnCount(9);
    ui->tableProductosCompra->setHorizontalHeaderLabels(
        QStringList()
        << "Código"
        << "Producto"
        << "Categoría"
        << "Unidad"
        << "Stock"
        << "Stock seg."
        << "Punto reorden"
        << "Estado"
        << "Costo unit."
        );

    ui->tableProductosCompra->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableProductosCompra->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableProductosCompra->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableProductosCompra->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //configura la tabla de orden en edicion
    ui->tableOrdenEdicion->setColumnCount(6);
    ui->tableOrdenEdicion->setHorizontalHeaderLabels(
        QStringList()
        << "Código"
        << "Producto"
        << "Unidad"
        << "Cantidad"
        << "Costo unit."
        << "Subtotal"
        );

    ui->tableOrdenEdicion->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableOrdenEdicion->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableOrdenEdicion->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableOrdenEdicion->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //configura la tabla de ordenes registradas
    ui->tableOrdenesRegistradas->setColumnCount(5);
    ui->tableOrdenesRegistradas->setHorizontalHeaderLabels(
        QStringList()
        << "ID"
        << "Fecha"
        << "Proveedor"
        << "Estado de la orden"
        << "Total"
        );

    ui->tableOrdenesRegistradas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableOrdenesRegistradas->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableOrdenesRegistradas->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableOrdenesRegistradas->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //configura el rango de cantidad para la orden de compra
    ui->spinCantidadCompra->setMinimum(1);
    ui->spinCantidadCompra->setMaximum(999);
    ui->spinCantidadCompra->setValue(1);

    ui->labelTotalOrden->setText("Total: $0");
}


//////////// TABLA DE PRODUCTOS PARA COMPRA //////////////

void MainWindow::cargarTablaProductosCompra() {
    if(!baseDatos.estaAbierta()) {
        ui->tableProductosCompra->setRowCount(0);
        return;
    }

    QString proveedor = ui->comboProveedorCompra->currentText();

    QVector<ProductoDB> productos = baseDatos.obtenerProductos("Todas", proveedor, "");

    ui->tableProductosCompra->setRowCount(0);

    for(int i = 0; i < productos.size(); i++) {
        ProductoDB prod = productos[i];

        double demandaEstimada = baseDatos.calcularDemandaPromedioMovil(prod.getCodigo(), 7);
        int puntoReorden = qCeil(demandaEstimada * prod.getLeadTime() + prod.getStockSeguridad());

        QString estado = "OK";

        if(prod.getStock() <= puntoReorden) {
            estado = "CRÍTICO";
        }

        int fila = ui->tableProductosCompra->rowCount();
        ui->tableProductosCompra->insertRow(fila);

        ui->tableProductosCompra->setItem(fila, 0, new QTableWidgetItem(prod.getCodigo()));
        ui->tableProductosCompra->setItem(fila, 1, new QTableWidgetItem(prod.getNombre()));
        ui->tableProductosCompra->setItem(fila, 2, new QTableWidgetItem(prod.getCategoria()));
        ui->tableProductosCompra->setItem(fila, 3, new QTableWidgetItem(prod.getUnidad()));
        ui->tableProductosCompra->setItem(fila, 4, new QTableWidgetItem(QString::number(prod.getStock())));
        ui->tableProductosCompra->setItem(fila, 5, new QTableWidgetItem(QString::number(prod.getStockSeguridad())));
        ui->tableProductosCompra->setItem(fila, 6, new QTableWidgetItem(QString::number(puntoReorden)));

        QTableWidgetItem *itemEstado = new QTableWidgetItem(estado);

        if(estado == "CRÍTICO") {
            itemEstado->setForeground(Qt::red);
        } else {
            itemEstado->setForeground(Qt::darkGreen);
        }

        ui->tableProductosCompra->setItem(fila, 7, itemEstado);
        ui->tableProductosCompra->setItem(fila, 8, new QTableWidgetItem(QString::number(prod.getCosto())));
    }
}

void MainWindow::on_comboProveedorCompra_currentTextChanged(const QString& texto) {
    Q_UNUSED(texto);
    cargarTablaProductosCompra();
}


//////////// ORDEN EN EDICION //////////////

ProductoDB MainWindow::obtenerProductoSeleccionadoCompra() const {
    int fila = ui->tableProductosCompra->currentRow();

    if(fila < 0) {
        return ProductoDB();
    }

    QString proveedor = ui->comboProveedorCompra->currentText();

    QString codigo = ui->tableProductosCompra->item(fila, 0)->text();
    QString nombre = ui->tableProductosCompra->item(fila, 1)->text();
    QString categoria = ui->tableProductosCompra->item(fila, 2)->text();
    QString unidad = ui->tableProductosCompra->item(fila, 3)->text();
    int stock = ui->tableProductosCompra->item(fila, 4)->text().toInt();
    int stockSeguridad = ui->tableProductosCompra->item(fila, 5)->text().toInt();
    int costo = ui->tableProductosCompra->item(fila, 8)->text().toInt();

    return ProductoDB(codigo, nombre, categoria, proveedor, unidad, stock, stockSeguridad, costo, 0, 0);
}

void MainWindow::actualizarTablaOrdenCompra() {
    ui->tableOrdenEdicion->setRowCount(0);

    for(int i = 0; i < ordenCompra.size(); i++) {
        ItemCompra item = ordenCompra[i];

        int fila = ui->tableOrdenEdicion->rowCount();
        ui->tableOrdenEdicion->insertRow(fila);

        ui->tableOrdenEdicion->setItem(fila, 0, new QTableWidgetItem(item.getProducto().getCodigo()));
        ui->tableOrdenEdicion->setItem(fila, 1, new QTableWidgetItem(item.getProducto().getNombre()));
        ui->tableOrdenEdicion->setItem(fila, 2, new QTableWidgetItem(item.getProducto().getUnidad()));
        ui->tableOrdenEdicion->setItem(fila, 3, new QTableWidgetItem(QString::number(item.getCantidad())));
        ui->tableOrdenEdicion->setItem(fila, 4, new QTableWidgetItem(QString::number(item.getCostoUnitario())));
        ui->tableOrdenEdicion->setItem(fila, 5, new QTableWidgetItem(QString::number(item.calcularSubtotal())));
    }

    ui->labelTotalOrden->setText("Total: $" + QString::number(calcularTotalOrdenCompra()));
}

int MainWindow::calcularTotalOrdenCompra() const {
    int total = 0;

    for(int i = 0; i < ordenCompra.size(); i++) {
        total += ordenCompra[i].calcularSubtotal();
    }

    return total;
}

int MainWindow::buscarProductoEnOrdenCompra(const QString& codigo) const {
    for(int i = 0; i < ordenCompra.size(); i++) {
        if(ordenCompra[i].getProducto().getCodigo() == codigo) {
            return i;
        }
    }

    return -1;
}


//////////// BOTONES DE ORDEN EN EDICION //////////////

void MainWindow::on_btnAgregarProductoCompra_clicked() {
    if(!baseDatos.estaAbierta()) {
        QMessageBox::warning(this, "Base no cargada", "Primero debes cargar una base de datos.");
        return;
    }

    if(ui->comboProveedorCompra->currentText() == "Todos") {
        QMessageBox::warning(this, "Proveedor no seleccionado", "Selecciona un proveedor antes de agregar productos a la orden.");
        return;
    }

    if(ui->tableProductosCompra->currentRow() < 0) {
        QMessageBox::warning(this, "Producto no seleccionado", "Selecciona un producto antes de agregarlo a la orden.");
        return;
    }

    ProductoDB producto = obtenerProductoSeleccionadoCompra();

    if(producto.getCodigo().isEmpty()) {
        QMessageBox::warning(this, "Producto inválido", "No se pudo leer el producto seleccionado.");
        return;
    }

    if(!ordenCompra.isEmpty()) {
        QString proveedorOrden = ordenCompra[0].getProducto().getProveedor();

        if(producto.getProveedor() != proveedorOrden) {
            QMessageBox::warning(
                this,
                "Proveedor distinto",
                "La orden actual ya tiene productos de otro proveedor. Registra o vacía la orden antes de cambiar de proveedor."
                );
            return;
        }
    }

    int cantidad = ui->spinCantidadCompra->value();

    if(cantidad <= 0) {
        QMessageBox::warning(this, "Cantidad inválida", "La cantidad debe ser mayor que cero.");
        return;
    }

    int indice = buscarProductoEnOrdenCompra(producto.getCodigo());

    if(indice >= 0) {
        ordenCompra[indice].aumentarCantidad(cantidad);
    } else {
        ordenCompra.push_back(ItemCompra(producto, cantidad));
    }

    actualizarTablaOrdenCompra();
}

void MainWindow::on_btnGenerarPedidoROP_clicked() {
    if(!baseDatos.estaAbierta()) {
        QMessageBox::warning(this, "Base no cargada", "Primero debes cargar una base de datos.");
        return;
    }

    QString proveedor = ui->comboProveedorCompra->currentText();

    if(proveedor == "Todos") {
        QMessageBox::warning(this, "Proveedor no seleccionado", "Selecciona un proveedor para generar un pedido por punto de reorden.");
        return;
    }

    if(!ordenCompra.isEmpty()) {
        QString proveedorOrden = ordenCompra[0].getProducto().getProveedor();

        if(proveedor != proveedorOrden) {
            QMessageBox::warning(
                this,
                "Proveedor distinto",
                "La orden en edición pertenece a otro proveedor. Regístrala o vacíala antes de generar una nueva."
                );
            return;
        }
    }

    QVector<ProductoDB> productos = baseDatos.obtenerProductos("Todas", proveedor, "");

    int agregados = 0;

    for(int i = 0; i < productos.size(); i++) {
        ProductoDB prod = productos[i];

        double demandaEstimada = baseDatos.calcularDemandaPromedioMovil(prod.getCodigo(), 7);
        int puntoReorden = qCeil(demandaEstimada * prod.getLeadTime() + prod.getStockSeguridad());

        if(prod.getStock() <= puntoReorden) {
            int cantidadSugerida = puntoReorden - prod.getStock() + prod.getStockSeguridad();

            if(cantidadSugerida <= 0) {
                cantidadSugerida = 1;
            }

            int indice = buscarProductoEnOrdenCompra(prod.getCodigo());

            if(indice >= 0) {
                ordenCompra[indice].aumentarCantidad(cantidadSugerida);
            } else {
                ordenCompra.push_back(ItemCompra(prod, cantidadSugerida));
            }

            agregados++;
        }
    }

    actualizarTablaOrdenCompra();

    if(agregados == 0) {
        QMessageBox::information(this, "Sin sugerencias", "No hay productos bajo el punto de reorden para este proveedor.");
    } else {
        QMessageBox::information(this, "Pedido generado", "Se agregaron productos sugeridos a la orden en edición.");
    }
}

void MainWindow::on_btnQuitarDeOrden_clicked() {
    int fila = ui->tableOrdenEdicion->currentRow();

    if(fila < 0) {
        QMessageBox::warning(this, "Producto no seleccionado", "Selecciona un producto de la orden para quitarlo.");
        return;
    }

    if(fila >= ordenCompra.size()) {
        return;
    }

    ordenCompra.removeAt(fila);
    actualizarTablaOrdenCompra();
}

void MainWindow::on_btnCrearOrden_clicked() {
    if(!baseDatos.estaAbierta()) {
        QMessageBox::warning(this, "Base no cargada", "Primero debes cargar una base de datos.");
        return;
    }

    if(ordenCompra.isEmpty()) {
        QMessageBox::warning(this, "Orden vacía", "No hay productos en la orden.");
        return;
    }

    QString error;

    if(!baseDatos.registrarOrdenCompra(ordenCompra, error)) {
        QMessageBox::critical(this, "Error al crear orden", error);
        return;
    }

    ordenCompra.clear();

    actualizarTablaOrdenCompra();
    actualizarTablasDesdeBaseDatos();

    QMessageBox::information(this, "Orden creada", "La orden de compra quedó registrada como Pendiente.");
}


//////////// ORDENES REGISTRADAS //////////////

void MainWindow::cargarTablaOrdenesCompra() {
    if(!baseDatos.estaAbierta()) {
        ui->tableOrdenesRegistradas->setRowCount(0);
        return;
    }

    QVector<OrdenCompra> ordenes = baseDatos.obtenerOrdenesCompra();

    ui->tableOrdenesRegistradas->setRowCount(0);

    for(int i = 0; i < ordenes.size(); i++) {
        OrdenCompra orden = ordenes[i];

        int fila = ui->tableOrdenesRegistradas->rowCount();
        ui->tableOrdenesRegistradas->insertRow(fila);

        ui->tableOrdenesRegistradas->setItem(fila, 0, new QTableWidgetItem(QString::number(orden.getId())));
        ui->tableOrdenesRegistradas->setItem(fila, 1, new QTableWidgetItem(orden.getFecha()));
        ui->tableOrdenesRegistradas->setItem(fila, 2, new QTableWidgetItem(orden.getProveedor()));

        QTableWidgetItem *itemEstado = new QTableWidgetItem(orden.getEstado());

        if(orden.getEstado() == "Pendiente") {
            itemEstado->setForeground(Qt::darkYellow);
        } else if(orden.getEstado() == "Recibida") {
            itemEstado->setForeground(Qt::darkGreen);
        } else {
            itemEstado->setForeground(Qt::red);
        }

        ui->tableOrdenesRegistradas->setItem(fila, 3, itemEstado);
        ui->tableOrdenesRegistradas->setItem(fila, 4, new QTableWidgetItem(QString::number(orden.getTotal())));
    }
}

int MainWindow::obtenerIdOrdenSeleccionadaCompra() const {
    int fila = ui->tableOrdenesRegistradas->currentRow();

    if(fila < 0) {
        return -1;
    }

    QTableWidgetItem *itemId = ui->tableOrdenesRegistradas->item(fila, 0);

    if(itemId == nullptr) {
        return -1;
    }

    return itemId->text().toInt();
}

void MainWindow::on_btnOrdenRecibida_clicked() {
    if(!baseDatos.estaAbierta()) {
        QMessageBox::warning(this, "Base no cargada", "Primero debes cargar una base de datos.");
        return;
    }

    int idCompra = obtenerIdOrdenSeleccionadaCompra();

    if(idCompra <= 0) {
        QMessageBox::warning(this, "Orden no seleccionada", "Selecciona una orden registrada.");
        return;
    }

    QString error;

    if(!baseDatos.marcarOrdenCompraRecibida(idCompra, error)) {
        QMessageBox::critical(this, "Error al recibir orden", error);
        return;
    }

    actualizarTablasDesdeBaseDatos();

    QMessageBox::information(this, "Orden recibida", "La orden fue marcada como recibida y el stock fue actualizado.");
}

void MainWindow::on_btnCancelarOrden_clicked() {
    if(!baseDatos.estaAbierta()) {
        QMessageBox::warning(this, "Base no cargada", "Primero debes cargar una base de datos.");
        return;
    }

    int idCompra = obtenerIdOrdenSeleccionadaCompra();

    if(idCompra <= 0) {
        QMessageBox::warning(this, "Orden no seleccionada", "Selecciona una orden registrada.");
        return;
    }

    QString error;

    if(!baseDatos.cancelarOrdenCompra(idCompra, error)) {
        QMessageBox::critical(this, "Error al cancelar orden", error);
        return;
    }

    actualizarTablasDesdeBaseDatos();

    QMessageBox::information(this, "Orden cancelada", "La orden fue cancelada correctamente.");
}
