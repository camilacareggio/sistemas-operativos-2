package main

import (
	"encoding/json"
	"log"
	"net/http"
	"runtime"
)

// struct del resumen de los sensores
type SensorSummary struct {
	Processing float64 `json:"processing"`
	FreeMemory float64 `json:"free_memory"`
	Swap       float64 `json:"swap"`
}

// Estructura global para almacenar el resumen de los sensores
var summary SensorSummary

func processingSubmitHandler(w http.ResponseWriter, r *http.Request) {
	// verificar el método HTTP utilizado
	if r.Method != http.MethodPost {
		w.WriteHeader(http.StatusMethodNotAllowed)
		return
	}

	// Agregar cálculo de procesamiento, memoria libre y swap
	summary.Processing = calcularProcesamiento()
	summary.FreeMemory = calcularMemoriaLibre()
	summary.Swap = calcularSwap()

	// responder con un mensaje de éxito
	response := map[string]string{
		"message": "Solicitud de procesamiento recibida correctamente",
	}
	jsonResponse, err := json.Marshal(response)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		return
	}
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	w.Write(jsonResponse)
}

// calcular el los cpus disponibles
func calcularProcesamiento() float64 {
	numCPU := float64(runtime.NumCPU())
	return numCPU
}

// calcular la memoria libre
func calcularMemoriaLibre() float64 {
	var memStats runtime.MemStats
	runtime.ReadMemStats(&memStats)
	freeMemory := float64(memStats.Frees)
	return freeMemory
}

// calcular el swap
func calcularSwap() float64 {
	var memStats runtime.MemStats
	runtime.ReadMemStats(&memStats)
	swap := float64(memStats.Sys - memStats.HeapSys)
	return swap
}

func processingSummaryHandler(w http.ResponseWriter, r *http.Request) {
	// verificar el método HTTP utilizado
	if r.Method != http.MethodGet {
		w.WriteHeader(http.StatusMethodNotAllowed)
		return
	}

	// responder con el resumen de los sensores en formato JSON
	jsonSummary, err := json.Marshal(summary)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		return
	}
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	w.Write(jsonSummary)
}

// catch-all handler for 404 Not Found
func notFoundHandler(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusNotFound)
	w.Header().Set("Content-Type", "application/json")
	errorMessage := map[string]string{
		"error":   "Not Found",
		"message": "This is a custom 404 error message.",
	}
	jsonError, _ := json.Marshal(errorMessage)
	w.Write(jsonError)
}

func main() {
	http.HandleFunc("/api/processing/submit", processingSubmitHandler)
	http.HandleFunc("/api/processing/summary", processingSummaryHandler)
	http.HandleFunc("/", notFoundHandler)

	// Iniciar el servidor en el puerto 9100
	log.Println("Iniciando el servidor en http://localhost:9100")
	log.Fatal(http.ListenAndServe(":9100", nil))
}
