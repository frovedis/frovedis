#python bipython sh

export PYTHONPATH=../main/python:$PYTHONPATH
export LD_LIBRARY_PATH=../main/lib:$LD_LIBRARY_PATH

COMMAND="mpirun -np 2 ../../server/frovedis_server"
#COMMAND="mpirun -np 1 $FROVEDIS_SERVER" 

if [ ! -d out ]; then
       mkdir out
fi

# --- Matrix Creation Demo ---
python crs_matrix_demo.py "$COMMAND"
python dense_matrix_demo.py "$COMMAND"
python df_demo.py "$COMMAND"

# --- ML Data Creation Demo ---
python frovedis_labeled_point_demo.py "$COMMAND"

# --- Wrapper Demo ---
python pblas_demo.py "$COMMAND"
python scalapack_demo.py "$COMMAND"
python sparse_svd_demo.py "$COMMAND"

# --- ML Demo ---
python lr_demo.py "$COMMAND"
python lnr_demo.py "$COMMAND"
python svm_demo.py "$COMMAND"
python dt_demo.py "$COMMAND"
python nbm_demo.py "$COMMAND"
python fm_demo.py "$COMMAND"
python kmeans_demo.py "$COMMAND"
python als_demo.py "$COMMAND"
