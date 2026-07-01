#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTableWidgetItem>
#include <cmath>


//////////// TABLA EN PESTAÑA INVENTARIO ////////////////

void MainWindow::cargarTablaInventario() {
    if(!baseDatos.estaAbierta()) {
        return;
    }

    QString categoria = ui->comboCatInventario->currentText();
    QString proveedor = ui->comboProvInventario->currentText();
    QString filtro = ui->lineFiltroInventario->text();

    QVector<ProductoDB> productos = baseDatos.obtenerProductos(categoria, proveedor, filtro);

    ui->tableInventario->setRowCount(0);

    int diasPromedioMovil = 7;

    for(int i = 0; i < productos.size(); i++) {
        ProductoDB prod = productos[i];

        double demandaEstimada = baseDatos.calcularDemandaPromedioMovil(prod.getCodigo(), diasPromedioMovil);
        double puntoReordenDecimal = demandaEstimada * prod.getLeadTime() + prod.getStockSeguridad();
        int puntoReorden = static_cast<int>(std::ceil(puntoReordenDecimal));

        QString estado = "OK";

        if(prod.getStock() <= puntoReorden) {
            estado = "CRÍTICO";
        }

        int fila = ui->tableInventario->rowCount();
        ui->tableInventario->insertRow(fila);

        ui->tableInventario->setItem(fila, 0, new QTableWidgetItem(prod.getCodigo()));
        ui->tableInventario->setItem(fila, 1, new QTableWidgetItem(prod.getNombre()));
        ui->tableInventario->setItem(fila, 2, new QTableWidgetItem(prod.getCategoria()));
        ui->tableInventario->setItem(fila, 3, new QTableWidgetItem(prod.getProveedor()));
        ui->tableInventario->setItem(fila, 4, new QTableWidgetItem(prod.getUnidad()));
        ui->tableInventario->setItem(fila, 5, new QTableWidgetItem(QString::number(prod.getStock())));
        ui->tableInventario->setItem(fila, 6, new QTableWidgetItem(QString::number(prod.getStockSeguridad())));
        ui->tableInventario->setItem(fila, 7, new QTableWidgetItem(QString::number(demandaEstimada, 'f', 2)));
        ui->tableInventario->setItem(fila, 8, new QTableWidgetItem(QString::number(prod.getLeadTime())));
        ui->tableInventario->setItem(fila, 9, new QTableWidgetItem(QString::number(puntoReorden)));

        QTableWidgetItem *itemEstado = new QTableWidgetItem(estado);

        if(estado == "CRÍTICO") {
            itemEstado->setForeground(Qt::red);
        } else {
            itemEstado->setForeground(Qt::darkGreen);
        }

        ui->tableInventario->setItem(fila, 10, itemEstado);
        ui->tableInventario->setItem(fila, 11, new QTableWidgetItem(QString::number(prod.getCosto())));
        ui->tableInventario->setItem(fila, 12, new QTableWidgetItem(QString::number(prod.getPrecioVenta())));
    }
}

//actualiza la tabla cuando se filtra por categoria
void MainWindow::on_comboCatInventario_currentTextChanged(const QString& texto) {
    Q_UNUSED(texto);
    cargarTablaInventario();
}

//actualiza la tabla cuando se filtra por proveedor
void MainWindow::on_comboProvInventario_currentTextChanged(const QString& texto) {
    Q_UNUSED(texto);
    cargarTablaInventario();
}

//actualiza la tabla cuando se escribe algo en el filtro
void MainWindow::on_lineFiltroInventario_textChanged(const QString& texto) {
    Q_UNUSED(texto);
    cargarTablaInventario();
}
