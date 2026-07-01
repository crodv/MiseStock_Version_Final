#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTableWidgetItem>
#include <QMessageBox>
#include <QHeaderView>
#include <QAbstractItemView>

#include <QPrinter>
#include <QTextDocument>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QPageSize>

//////////////////////////////////////////////////////////////////////////////
////////////////// ESTA SECCION FUE HECHA POR CLAUDIO R///////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////// TABLA EN PESTAÑA VENTAS ////////////////

void MainWindow::cargarTablaProductosVenta() {
    if(!baseDatos.estaAbierta()) {
        return;
    }

    //lee los filtros actuales de la pestaña venta
    QString categoria = ui->comboCatVenta->currentText();
    QString filtro = ui->lineFiltroVenta->text();

    //en venta no se filtra por proveedor, por eso se envia todos
    QVector<ProductoDB> productos = baseDatos.obtenerProductos(categoria, "Todos", filtro);

    ui->tableProductosVenta->setRowCount(0);

    for(int i = 0; i < productos.size(); i++) {
        int fila = ui->tableProductosVenta->rowCount();
        ui->tableProductosVenta->insertRow(fila);

        //este stock disponible descuenta lo que ya esta en el carro
        int stockDisponible = productos[i].getStock() - cantidadProductoEnCarro(productos[i].getCodigo());

        if(stockDisponible < 0) {
            stockDisponible = 0;
        }

        ui->tableProductosVenta->setItem(fila, 0, new QTableWidgetItem(productos[i].getCodigo()));
        ui->tableProductosVenta->setItem(fila, 1, new QTableWidgetItem(productos[i].getNombre()));
        ui->tableProductosVenta->setItem(fila, 2, new QTableWidgetItem(productos[i].getCategoria()));
        ui->tableProductosVenta->setItem(fila, 3, new QTableWidgetItem(productos[i].getProveedor()));
        ui->tableProductosVenta->setItem(fila, 4, new QTableWidgetItem(productos[i].getUnidad()));
        ui->tableProductosVenta->setItem(fila, 5, new QTableWidgetItem(QString::number(stockDisponible)));
        ui->tableProductosVenta->setItem(fila, 6, new QTableWidgetItem(QString::number(productos[i].getPrecioVenta())));
    }
}

//este metodo es para que se actualice la tabla al filtrar por categoria
void MainWindow::on_comboCatVenta_currentTextChanged(const QString& texto) {
    Q_UNUSED(texto);
    cargarTablaProductosVenta();
}

//y este metodo es para que se actualice la tabla al escribir algo en el filtro
void MainWindow::on_lineFiltroVenta_textChanged(const QString& texto) {
    Q_UNUSED(texto);
    cargarTablaProductosVenta();
}


//////////// TABLA DEL CARRO DE VENTA ////////////////

void MainWindow::configurarTablaCarroVenta() {
    //esto hace que al seleccionar se seleccione la fila completa
    ui->tableCarroVenta->setSelectionBehavior(QAbstractItemView::SelectRows);

    //esto permite seleccionar solo una fila a la vez
    ui->tableCarroVenta->setSelectionMode(QAbstractItemView::SingleSelection);

    //esto evita que el usuario edite directamente la tabla
    ui->tableCarroVenta->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //esto ajusta las columnas para ocupar el ancho disponible
    ui->tableCarroVenta->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

ProductoVenta MainWindow::obtenerProductoSeleccionadoVenta() const {
    //este metodo lee la fila seleccionada en la tabla de productos disponibles
    int fila = ui->tableProductosVenta->currentRow();

    if(fila < 0) {
        return ProductoVenta();
    }

    QString codigo = ui->tableProductosVenta->item(fila, 0)->text();
    QString nombre = ui->tableProductosVenta->item(fila, 1)->text();
    QString categoria = ui->tableProductosVenta->item(fila, 2)->text();
    QString proveedor = ui->tableProductosVenta->item(fila, 3)->text();
    QString unidad = ui->tableProductosVenta->item(fila, 4)->text();
    int stock = ui->tableProductosVenta->item(fila, 5)->text().toInt();
    int precioVenta = ui->tableProductosVenta->item(fila, 6)->text().toInt();

    return ProductoVenta(codigo, nombre, categoria, proveedor, unidad, stock, precioVenta);
}

void MainWindow::actualizarTablaCarroVenta() {
    //este metodo limpia y vuelve a llenar la tabla visual del carro
    ui->tableCarroVenta->setRowCount(0);

    for(int i = 0; i < carroVenta.size(); i++) {
        int fila = ui->tableCarroVenta->rowCount();
        ui->tableCarroVenta->insertRow(fila);

        ItemCarro item = carroVenta[i];

        ui->tableCarroVenta->setItem(fila, 0, new QTableWidgetItem(item.getProducto().getCodigo()));
        ui->tableCarroVenta->setItem(fila, 1, new QTableWidgetItem(item.getProducto().getNombre()));
        ui->tableCarroVenta->setItem(fila, 2, new QTableWidgetItem(QString::number(item.getCantidad())));
        ui->tableCarroVenta->setItem(fila, 3, new QTableWidgetItem(QString::number(item.getPrecioUnitario())));
        ui->tableCarroVenta->setItem(fila, 4, new QTableWidgetItem(QString::number(item.calcularSubtotal())));
    }

    //este label muestra el total actual del carro
    ui->labelTotalVenta->setText("Total: $" + QString::number(calcularTotalCarro()));
}

int MainWindow::calcularTotalCarro() const {
    //este metodo suma los subtotales de todos los items del carro
    int total = 0;

    for(int i = 0; i < carroVenta.size(); i++) {
        total += carroVenta[i].calcularSubtotal();
    }

    return total;
}

int MainWindow::buscarProductoEnCarro(const QString& codigo) const {
    //este metodo busca si un producto ya existe dentro del carro
    for(int i = 0; i < carroVenta.size(); i++) {
        if(carroVenta[i].getProducto().getCodigo() == codigo) {
            return i;
        }
    }

    return -1;
}

int MainWindow::cantidadProductoEnCarro(const QString& codigo) const {
    //este metodo obtiene la cantidad actual de un producto dentro del carro
    int cantidad = 0;

    for(int i = 0; i < carroVenta.size(); i++) {
        if(carroVenta[i].getProducto().getCodigo() == codigo) {
            cantidad += carroVenta[i].getCantidad();
        }
    }

    return cantidad;
}


//////////// BOTONES DEL CARRO DE VENTA ////////////////

void MainWindow::on_btnAgregarAlCarro_clicked() {
    //este metodo se ejecuta al presionar el boton agregar al carro
    if(!baseDatos.estaAbierta()) {
        QMessageBox::warning(this, "Base no cargada", "Primero debes cargar una base de datos.");
        return;
    }

    int filaSeleccionada = ui->tableProductosVenta->currentRow();

    if(filaSeleccionada < 0) {
        QMessageBox::warning(this, "Producto no seleccionado", "Selecciona un producto antes de agregarlo al carro.");
        return;
    }

    ProductoVenta producto = obtenerProductoSeleccionadoVenta();
    int cantidad = ui->spinBoxCantidadVenta->value();

    if(cantidad <= 0) {
        QMessageBox::warning(this, "Cantidad inválida", "La cantidad debe ser mayor a cero.");
        return;
    }

    if(cantidad > producto.getStock()) {
        QMessageBox::warning(this, "Stock insuficiente", "La cantidad solicitada supera el stock disponible.");
        return;
    }

    int indice = buscarProductoEnCarro(producto.getCodigo());

    //si el producto ya existe en el carro, solo aumenta su cantidad
    if(indice >= 0) {
        carroVenta[indice].aumentarCantidad(cantidad);
    } else {
        carroVenta.push_back(ItemCarro(producto, cantidad));
    }

    actualizarTablaCarroVenta();
    cargarTablaProductosVenta();
}

void MainWindow::on_btnQuitarCarroVenta_clicked() {
    //este metodo quita del carro el item seleccionado
    int fila = ui->tableCarroVenta->currentRow();

    if(fila < 0) {
        QMessageBox::warning(this, "Producto no seleccionado", "Selecciona un producto del carro para quitarlo.");
        return;
    }

    if(fila >= carroVenta.size()) {
        return;
    }

    carroVenta.removeAt(fila);

    actualizarTablaCarroVenta();
    cargarTablaProductosVenta();
}

void MainWindow::on_btnConfirmarVenta_clicked() {
    //este metodo guarda la venta en sqlite y descuenta el stock real
    if(!baseDatos.estaAbierta()) {
        QMessageBox::warning(this, "Base no cargada", "Primero debes cargar una base de datos.");
        return;
    }

    if(carroVenta.isEmpty()) {
        QMessageBox::warning(this, "Carro vacío", "No hay productos en el carro.");
        return;
    }

    QString error;

    if(!baseDatos.registrarVenta(carroVenta, error)) {
        QMessageBox::critical(this, "Error al confirmar venta", error);
        return;
    }

    QString rutaPDF;
    QString errorPDF;

    //se genera el vale de venta PDF
    generarValeVentaPDF(carroVenta, rutaPDF, errorPDF);

    carroVenta.clear();

    actualizarTablaCarroVenta();
    actualizarTablasDesdeBaseDatos();

    QMessageBox::information(this, "Venta confirmada", "La venta fue registrada correctamente.");
}

bool MainWindow::generarValeVentaPDF(const QVector<ItemCarro>& items, QString& rutaPDF, QString& error) {
    if(items.isEmpty()) {
        return false;
    }

    QString carpetaDocumentos = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    if(carpetaDocumentos.isEmpty()) {
        carpetaDocumentos = QDir::homePath();
    }

    QDir dir(carpetaDocumentos + "/MiseStock/ValesVenta");

    if(!dir.exists()) {
        if(!dir.mkpath(".")) {
            error = "No se pudo crear la carpeta de vales.";
            return false;
        }
    }

    QString fechaArchivo = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    rutaPDF = dir.filePath("vale_venta_" + fechaArchivo + ".pdf");

    int total = 0;

    QString html;
    html += "<html>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<style>";
    html += "body { font-family: Arial; font-size: 11pt; }";
    html += "h1 { text-align: center; }";
    html += "table { width: 100%; border-collapse: collapse; margin-top: 15px; }";
    html += "th, td { border: 1px solid #444; padding: 6px; }";
    html += "th { background-color: #dddddd; }";
    html += ".derecha { text-align: right; }";
    html += ".centro { text-align: center; }";
    html += ".total { font-size: 14pt; font-weight: bold; text-align: right; margin-top: 20px; }";
    html += "</style>";
    html += "</head>";
    html += "<body>";

    html += "<h1>Vale de venta</h1>";
    html += "<p><b>Fecha:</b> " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "</p>";

    html += "<table>";
    html += "<tr>";
    html += "<th>Código</th>";
    html += "<th>Producto</th>";
    html += "<th>Unidad</th>";
    html += "<th>Cantidad</th>";
    html += "<th>Precio unit.</th>";
    html += "<th>Subtotal</th>";
    html += "</tr>";

    for(int i = 0; i < items.size(); i++) {
        ItemCarro item = items[i];
        ProductoVenta producto = item.getProducto();

        total += item.calcularSubtotal();

        html += "<tr>";
        html += "<td>" + producto.getCodigo().toHtmlEscaped() + "</td>";
        html += "<td>" + producto.getNombre().toHtmlEscaped() + "</td>";
        html += "<td class='centro'>" + producto.getUnidad().toHtmlEscaped() + "</td>";
        html += "<td class='derecha'>" + QString::number(item.getCantidad()) + "</td>";
        html += "<td class='derecha'>$" + QString::number(item.getPrecioUnitario()) + "</td>";
        html += "<td class='derecha'>$" + QString::number(item.calcularSubtotal()) + "</td>";
        html += "</tr>";
    }

    html += "</table>";
    html += "<p class='total'>Total: $" + QString::number(total) + "</p>";

    html += "</body>";
    html += "</html>";

    QTextDocument documento;
    documento.setHtml(html);

    QPrinter impresora(QPrinter::HighResolution);
    impresora.setOutputFormat(QPrinter::PdfFormat);
    impresora.setOutputFileName(rutaPDF);
    impresora.setPageSize(QPageSize(QPageSize::A4));

    documento.print(&impresora);

    if(!QFile::exists(rutaPDF)) {
        error = "No se pudo generar el archivo PDF.";
        return false;
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
/////////////////////////// HECHA POR EL RESTO DEL GRUPO ////////////////////
////////////////////////////////////////////////////////////////////////////

void MainWindow::on_tableProductosVenta_itemSelectionChanged() {
    // Obtenemos el producto que el usuario acaba de seleccionar
    ProductoVenta producto = obtenerProductoSeleccionadoVenta();

    // Si no seleccionó nada (o deseleccionó), lo dejamos por defecto
    if(producto.getCodigo().isEmpty()) {
        ui->spinBoxCantidadVenta->setMinimum(1);
        ui->spinBoxCantidadVenta->setMaximum(999);
        ui->spinBoxCantidadVenta->setValue(1);
        return;
    }

    // Calculamos cuánto stock real le queda restando lo que ya echó al carro
    int stockDisponible = producto.getStock(); //- cantidadProductoEnCarro(producto.getCodigo());

    // Si se quedó sin stock, las flechitas se bloquean en 0
    if(stockDisponible <= 0) {
        ui->spinBoxCantidadVenta->setMinimum(0);
        ui->spinBoxCantidadVenta->setMaximum(0);
        ui->spinBoxCantidadVenta->setValue(0);
    }
    // Si hay stock, el máximo de las flechitas será el stock disponible
    else {
        ui->spinBoxCantidadVenta->setMinimum(1);
        ui->spinBoxCantidadVenta->setMaximum(stockDisponible);
        ui->spinBoxCantidadVenta->setValue(1);
    }
}
