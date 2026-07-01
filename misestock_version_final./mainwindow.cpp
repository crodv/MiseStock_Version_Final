#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle("MiseStock");

    //configura tablas principales de la aplicacion
    configurarTablas();

    //configura los combos fijos del historial
    configurarCombosHistorial();

    //configura la pestaña de analisis
    configurarAnalisis();

    //configura la pestaña de compra
    configurarCompra();

    //deja la pestaña de analisis con valores iniciales
    cargarDatosAnalisis();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::configurarTablas() {
    //tabla inventario
    ui->tableInventario->setColumnCount(13);
    ui->tableInventario->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableInventario->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableInventario->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //como inventario tiene muchas columnas, conviene permitir scroll horizontal
    ui->tableInventario->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableInventario->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    //tabla productos venta
    ui->tableProductosVenta->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableProductosVenta->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableProductosVenta->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableProductosVenta->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //configura tablas de otras pestañas
    configurarTablaCarroVenta();
    configurarTablaHistorialVentas();
    configurarTablaHistorialCompras();

    //configura valores iniciales de venta
    ui->spinBoxCantidadVenta->setMinimum(1);
    ui->spinBoxCantidadVenta->setMaximum(999);
    ui->labelTotalVenta->setText("Total: $0");
}
//actualiza las vistas que dependen de la base de datos
void MainWindow::actualizarTablasDesdeBaseDatos() {
    cargarTablaInventario();
    cargarTablaProductosVenta();
    cargarTablaHistorialVentas();
    cargarTablaHistorialCompras();
    cargarDatosAnalisis();

    cargarTablaProductosCompra();
    cargarTablaOrdenesCompra();
}
