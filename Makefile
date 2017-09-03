all:
	@echo "Este makefile solo borra los archivos objeto y ejecutables. No compila nada. Para compilar, ver el archivo fuente de cada ejemplo."

clean:
	rm -Rf *.o *.a *.so *.exe a.out 
