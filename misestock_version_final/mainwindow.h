#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

#include "database.h"
#include "clases.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DataBase baseDatos;
    QVector<ItemCarro> carroVenta;
    QVector<ItemCompra> ordenCompra;

    //metodos generales
    void configurarTablas();
    void actualizarTablasDesdeBaseDatos();

    //metodos de la pestaña Configuracion
    void cargarCombosDesdeBaseDatos();

    //metodos de la pestaña Inventario
    void cargarTablaInventario();

    //metodos de la pestaña Venta
    void cargarTablaProductosVenta();
    void configurarTablaCarroVenta();
    void actualizarTablaCarroVenta();
    ProductoVenta obtenerProductoSeleccionadoVenta() const;
    int calcularTotalCarro() const;
    int buscarProductoEnCarro(const QString& codigo) const;
    int cantidadProductoEnCarro(const QString& codigo) const;
    //metodo para generar el vale PDF de una venta
    bool generarValeVentaPDF(const QVector<ItemCarro>& items, QString& rutaPDF, QString& error);

    //metodos de la pestaña Compra
    void configurarCompra();
    void cargarTablaProductosCompra();
    void actualizarTablaOrdenCompra();
    void cargarTablaOrdenesCompra();
    ProductoDB obtenerProductoSeleccionadoCompra() const;
    int calcularTotalOrdenCompra() const;
    int buscarProductoEnOrdenCompra(const QString& codigo) const;
    int obtenerIdOrdenSeleccionadaCompra() const;


    //metodos de la pestaña historial
    void configurarCombosHistorial();
    void configurarTablaHistorialVentas();
    void configurarTablaHistorialCompras();
    void cargarTablaHistorialVentas();
    void cargarTablaHistorialCompras();
    //metodo para generar PDF de historial
    bool generarHistorialPDF(const QVector<HistorialVenta>& historial,
                             const QString& titulo,
                             const QString& periodo,
                             const QString& carpetaDestino,
                             const QString& prefijoArchivo,
                             const QString& nombreColumnaUnitario,
                             QString& rutaPDF,
                             QString& error);

    //metodos de la pestaña Analisis
    void configurarAnalisis();
    void cargarDatosAnalisis();



private slots:
    //slots de la pestaña Configuracion
    void on_btnCargarBaseDatos_clicked();

    //slots de la pestaña Inventario
    void on_comboCatInventario_currentTextChanged(const QString& texto);
    void on_comboProvInventario_currentTextChanged(const QString& texto);
    void on_lineFiltroInventario_textChanged(const QString& texto);

    //slots de la pestaña Venta
    void on_comboCatVenta_currentTextChanged(const QString& texto);
    void on_lineFiltroVenta_textChanged(const QString& texto);
    void on_btnAgregarAlCarro_clicked();
    void on_btnQuitarCarroVenta_clicked();
    void on_btnConfirmarVenta_clicked();
    void on_tableProductosVenta_itemSelectionChanged(); // Para controlar las flechitas de cantidad

    //slots de la pestaña Compra
    void on_comboProveedorCompra_currentTextChanged(const QString& texto);
    void on_btnAgregarProductoCompra_clicked();
    void on_btnGenerarPedidoROP_clicked();
    void on_btnQuitarDeOrden_clicked();
    void on_btnCrearOrden_clicked();
    void on_btnOrdenRecibida_clicked();
    void on_btnCancelarOrden_clicked();


    //slots de la pestaña historial
    void on_comboPeriodoHistVentas_currentTextChanged(const QString& texto);
    void on_comboPeriodoHistCompras_currentTextChanged(const QString& texto);
    void on_btnPDFHistVentas_clicked();
    void on_btnPDFHistCompras_clicked();

    //slots de la pestaña Analisis
    void on_comboPeriodoAnalisis_currentTextChanged(const QString& texto);
};
#endif // MAINWINDOW_H
