#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSignalBlocker>

void MainWindow::on_btnCargarBaseDatos_clicked() {
    //abre el selector de archivos para elegir una base sqlite
    QString ruta = QFileDialog::getOpenFileName(
        this,
        "Seleccionar base de datos SQLite",
        ".",
        "Base SQLite (*.sqlite *.db);;Todos los archivos (*.*)"
        );

    if(ruta.isEmpty()) {
        return;
    }

    //abre la base de datos seleccionada
    if(!baseDatos.abrirBaseDatos(ruta)) {
        QMessageBox::critical(this, "Error al abrir base de datos", baseDatos.getUltimoError());
        return;
    }

    //revisa que existan las tablas minimas necesarias
    if(!baseDatos.validarEstructura()) {
        QMessageBox::critical(this, "Base de datos no válida", baseDatos.getUltimoError());
        return;
    }

    //carga combos y tablas desde la base
    cargarCombosDesdeBaseDatos();
    actualizarTablasDesdeBaseDatos();

    QMessageBox::information(this, "Base cargada", "La base de datos fue cargada correctamente.");
}

void MainWindow::cargarCombosDesdeBaseDatos() {
    //bloquea senales para que no se actualicen tablas mientras se cargan combos
    QSignalBlocker bloqueaCategoriaInventario(ui->comboCatInventario);
    QSignalBlocker bloqueaProveedorInventario(ui->comboProvInventario);
    QSignalBlocker bloqueaCategoriaVenta(ui->comboCatVenta);
    QSignalBlocker bloqueaProveedorCompra(ui->comboProveedorCompra);

    ui->comboCatInventario->clear();
    ui->comboProvInventario->clear();
    ui->comboCatVenta->clear();
    ui->comboProveedorCompra->clear();

    ui->comboCatInventario->addItem("Todas");
    ui->comboProvInventario->addItem("Todos");
    ui->comboCatVenta->addItem("Todas");
    ui->comboProveedorCompra->addItem("Todos");

    //carga categorias para inventario y venta
    QVector<QString> categorias = baseDatos.obtenerCategorias();

    for(int i = 0; i < categorias.size(); i++) {
        ui->comboCatInventario->addItem(categorias[i]);
        ui->comboCatVenta->addItem(categorias[i]);
    }

    //carga proveedores para inventario y compra
    QVector<QString> proveedores = baseDatos.obtenerProveedores();

    for(int i = 0; i < proveedores.size(); i++) {
        ui->comboProvInventario->addItem(proveedores[i]);
        ui->comboProveedorCompra->addItem(proveedores[i]);
    }
}
