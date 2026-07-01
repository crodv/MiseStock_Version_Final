#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QSignalBlocker>
#include <QMessageBox>

#include <QtPrintSupport/QPrinter>
#include <QTextDocument>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QPageSize>

void MainWindow::configurarCombosHistorial() {
    QSignalBlocker bloqueaVentas(ui->comboPeriodoHistVentas);
    QSignalBlocker bloqueaCompras(ui->comboPeriodoHistCompras);

    ui->comboPeriodoHistVentas->clear();
    ui->comboPeriodoHistCompras->clear();

    ui->comboPeriodoHistVentas->addItem("Todos");
    ui->comboPeriodoHistVentas->addItem("Hoy");
    ui->comboPeriodoHistVentas->addItem("Últimos 7 días");
    ui->comboPeriodoHistVentas->addItem("Este mes");

    ui->comboPeriodoHistCompras->addItem("Todos");
    ui->comboPeriodoHistCompras->addItem("Hoy");
    ui->comboPeriodoHistCompras->addItem("Últimos 7 días");
    ui->comboPeriodoHistCompras->addItem("Este mes");
}

void MainWindow::configurarTablaHistorialVentas() {
    ui->tableHistorialVentas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableHistorialVentas->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableHistorialVentas->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableHistorialVentas->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::cargarTablaHistorialVentas() {
    QString periodo = ui->comboPeriodoHistVentas->currentText();
    QString textoPeriodo = (periodo == "Todos") ? "Totales" : periodo.toLower();

    // Actualizamos el titulo dinámicamente
    ui->labelHistorialVentas->setText("Historial de Ventas (" + textoPeriodo + ")");

    if(!baseDatos.estaAbierta()) {
        ui->tableHistorialVentas->setRowCount(0);
        return;
    }

    QVector<HistorialVenta> historial = baseDatos.obtenerHistorialVentas(periodo);
    ui->tableHistorialVentas->setRowCount(0);

    for(int i = 0; i < historial.size(); i++) {
        int fila = ui->tableHistorialVentas->rowCount();
        ui->tableHistorialVentas->insertRow(fila);
        ui->tableHistorialVentas->setItem(fila, 0, new QTableWidgetItem(historial[i].getFecha()));
        ui->tableHistorialVentas->setItem(fila, 1, new QTableWidgetItem(historial[i].getTipo()));
        ui->tableHistorialVentas->setItem(fila, 2, new QTableWidgetItem(historial[i].getCodigo()));
        ui->tableHistorialVentas->setItem(fila, 3, new QTableWidgetItem(historial[i].getProducto()));
        ui->tableHistorialVentas->setItem(fila, 4, new QTableWidgetItem(QString::number(historial[i].getCantidad())));
        ui->tableHistorialVentas->setItem(fila, 5, new QTableWidgetItem(QString::number(historial[i].getPrecioUnitario())));
        ui->tableHistorialVentas->setItem(fila, 6, new QTableWidgetItem(QString::number(historial[i].getSubtotal())));
        ui->tableHistorialVentas->setItem(fila, 7, new QTableWidgetItem(historial[i].getDescripcion()));
    }
}

void MainWindow::on_comboPeriodoHistVentas_currentTextChanged(const QString& texto) {
    Q_UNUSED(texto);
    cargarTablaHistorialVentas();
}


bool MainWindow::generarHistorialPDF(const QVector<HistorialVenta>& historial,
                                     const QString& titulo,
                                     const QString& periodo,
                                     const QString& carpetaDestino,
                                     const QString& prefijoArchivo,
                                     const QString& nombreColumnaUnitario,
                                     QString& rutaPDF,
                                     QString& error) {
    if(historial.isEmpty()) {
        return false;
    }

    QString carpetaDocumentos = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    if(carpetaDocumentos.isEmpty()) {
        carpetaDocumentos = QDir::homePath();
    }

    QDir dir(carpetaDocumentos + "/MiseStock/" + carpetaDestino);

    if(!dir.exists()) {
        if(!dir.mkpath(".")) {
            error = "No se pudo crear la carpeta del historial.";
            return false;
        }
    }

    QString fechaArchivo = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");
    rutaPDF = dir.filePath(prefijoArchivo + "_" + fechaArchivo + ".pdf");

    int total = 0;
    int cantidadTotal = 0;

    QString html;
    html += "<html>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<style>";
    html += "body { font-family: Arial; font-size: 10pt; }";
    html += "h1 { text-align: center; }";
    html += "p { margin: 4px 0; }";
    html += "table { width: 100%; border-collapse: collapse; margin-top: 15px; }";
    html += "th, td { border: 1px solid #444; padding: 5px; }";
    html += "th { background-color: #dddddd; }";
    html += ".derecha { text-align: right; }";
    html += ".centro { text-align: center; }";
    html += ".resumen { margin-top: 18px; font-size: 12pt; font-weight: bold; text-align: right; }";
    html += "</style>";
    html += "</head>";
    html += "<body>";

    html += "<h1>" + titulo.toHtmlEscaped() + "</h1>";
    html += "<p><b>Periodo:</b> " + periodo.toHtmlEscaped() + "</p>";
    html += "<p><b>Fecha de generación:</b> " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "</p>";

    html += "<table>";
    html += "<tr>";
    html += "<th>Fecha</th>";
    html += "<th>Tipo</th>";
    html += "<th>Código</th>";
    html += "<th>Producto</th>";
    html += "<th>Cantidad</th>";
    html += "<th>" + nombreColumnaUnitario.toHtmlEscaped() + "</th>";
    html += "<th>Subtotal</th>";
    html += "<th>Descripción</th>";
    html += "</tr>";

    for(int i = 0; i < historial.size(); i++) {
        HistorialVenta item = historial[i];

        cantidadTotal += item.getCantidad();
        total += item.getSubtotal();

        html += "<tr>";
        html += "<td>" + item.getFecha().toHtmlEscaped() + "</td>";
        html += "<td class='centro'>" + item.getTipo().toHtmlEscaped() + "</td>";
        html += "<td>" + item.getCodigo().toHtmlEscaped() + "</td>";
        html += "<td>" + item.getProducto().toHtmlEscaped() + "</td>";
        html += "<td class='derecha'>" + QString::number(item.getCantidad()) + "</td>";
        html += "<td class='derecha'>$" + QString::number(item.getPrecioUnitario()) + "</td>";
        html += "<td class='derecha'>$" + QString::number(item.getSubtotal()) + "</td>";
        html += "<td>" + item.getDescripcion().toHtmlEscaped() + "</td>";
        html += "</tr>";
    }

    html += "</table>";

    html += "<p class='resumen'>Cantidad total: " + QString::number(cantidadTotal) + "</p>";
    html += "<p class='resumen'>Total: $" + QString::number(total) + "</p>";

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

void MainWindow::on_btnPDFHistVentas_clicked() {
    if(!baseDatos.estaAbierta()) {
        QMessageBox::warning(this, "Base no cargada", "Primero se debe cargar una base de datos.");
        return;
    }

    QString periodo = ui->comboPeriodoHistVentas->currentText();
    QVector<HistorialVenta> historial = baseDatos.obtenerHistorialVentas(periodo);

    if(historial.isEmpty()) {
        QMessageBox::information(this, "Historial vacío", "No hay ventas para el periodo seleccionado.");
        return;
    }

    QString rutaPDF;
    QString error;

    if(!generarHistorialPDF(historial,
                             "Historial de ventas",
                             periodo,
                             "HistorialVentas",
                             "historial_ventas",
                             "Precio unit.",
                             rutaPDF,
                             error)) {
        QMessageBox::critical(this, "Error al generar PDF", error);
        return;
    }

    QMessageBox::information(this, "PDF generado", "El historial de ventas fue generado correctamente.\n\n" + rutaPDF);
}

void MainWindow::configurarTablaHistorialCompras() {
    ui->tableHistorialCompras->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableHistorialCompras->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableHistorialCompras->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableHistorialCompras->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::cargarTablaHistorialCompras() {
    QString periodo = ui->comboPeriodoHistCompras->currentText();
    QString textoPeriodo = (periodo == "Todos") ? "Totales" : periodo.toLower();

    // Actualizamos el titulo dinámicamente
    ui->labelHistorialCompras->setText("Historial de Compras (" + textoPeriodo + ")");

    if(!baseDatos.estaAbierta()) {
        ui->tableHistorialCompras->setRowCount(0);
        return;
    }

    // Pedimos los datos a la base de datos
    QVector<HistorialVenta> historial = baseDatos.obtenerHistorialCompras(periodo);
    ui->tableHistorialCompras->setRowCount(0);

    // Llenamos la tabla fila por fila
    for(int i = 0; i < historial.size(); i++) {
        int fila = ui->tableHistorialCompras->rowCount();
        ui->tableHistorialCompras->insertRow(fila);

        ui->tableHistorialCompras->setItem(fila, 0, new QTableWidgetItem(historial[i].getFecha()));
        ui->tableHistorialCompras->setItem(fila, 1, new QTableWidgetItem(historial[i].getTipo()));
        ui->tableHistorialCompras->setItem(fila, 2, new QTableWidgetItem(historial[i].getCodigo()));
        ui->tableHistorialCompras->setItem(fila, 3, new QTableWidgetItem(historial[i].getProducto()));
        ui->tableHistorialCompras->setItem(fila, 4, new QTableWidgetItem(QString::number(historial[i].getCantidad())));
        ui->tableHistorialCompras->setItem(fila, 5, new QTableWidgetItem(QString::number(historial[i].getPrecioUnitario()))); // Costo Unitario
        ui->tableHistorialCompras->setItem(fila, 6, new QTableWidgetItem(QString::number(historial[i].getSubtotal()))); // Subtotal
        ui->tableHistorialCompras->setItem(fila, 7, new QTableWidgetItem(historial[i].getDescripcion()));
    }
}

void MainWindow::on_comboPeriodoHistCompras_currentTextChanged(const QString& texto) {
    Q_UNUSED(texto);
    cargarTablaHistorialCompras();
}

void MainWindow::on_btnPDFHistCompras_clicked() {
    if(!baseDatos.estaAbierta()) {
        QMessageBox::warning(this, "Base no cargada", "Primero se debe cargar una base de datos.");
        return;
    }

    QString periodo = ui->comboPeriodoHistCompras->currentText();
    QVector<HistorialVenta> historial = baseDatos.obtenerHistorialCompras(periodo);

    if(historial.isEmpty()) {
        QMessageBox::information(this, "Historial vacío", "No hay compras para el periodo seleccionado.");
        return;
    }

    QString rutaPDF;
    QString error;

    if(!generarHistorialPDF(historial,
                             "Historial de compras",
                             periodo,
                             "HistorialCompras",
                             "historial_compras",
                             "Costo unit.",
                             rutaPDF,
                             error)) {
        QMessageBox::critical(this, "Error al generar PDF", error);
        return;
    }

    QMessageBox::information(this, "PDF generado", "El historial de compras fue generado correctamente.\n\n" + rutaPDF);
}
