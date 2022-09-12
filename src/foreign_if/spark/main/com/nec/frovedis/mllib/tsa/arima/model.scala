package com.nec.frovedis.mllib.tsa.arima;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.matrix.{DTYPE, FloatDvector, DoubleDvector}
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import org.apache.spark.sql.SparkSession
import scala.reflect.runtime.universe._
import org.apache.spark.sql.functions._
import org.apache.spark.sql.DataFrame
import org.apache.spark.sql.expressions._

class ARIMAModel(modelId:Int, dtype:Short, _endog_len:Long, freq:Int, 
                 end_index:Long, start_offset:Long)
    extends GenericModel(modelId, M_KIND.ARM) {
    override def release():Unit = {
        val fs = FrovedisServer.getServerInstance()
        JNISupport.releaseARIMAModel(fs.master_node, 
                                     this.modelId, this.dtype)
        val info = JNISupport.checkServerException()
        if (info != "") throw new java.rmi.ServerException(info)
    }
    def fittedvalues():DataFrame = {
        var fittedVals:Array[Double] = null
        val fs = FrovedisServer.getServerInstance()
        fittedVals = JNISupport.getFittedVector(fs.master_node, 
                                           this.modelId, this.dtype)
        val info = JNISupport.checkServerException()
        if (info != "") throw new java.rmi.ServerException(info)
        val spark = SparkSession.builder().getOrCreate()
        import spark.implicits._
        return ((this.start_offset to
                 this.end_index by 
                 this.freq) 
                zip fittedVals)
               .toDF("index", "")
    }

    def predict(start:Long = 0, end:Long = this._endog_len - 1):
        DataFrame = {
        var _start = start
        var _end = end
        if (_start < 0) {
            require (this._endog_len >= _start.abs,
                    s"The `start` argument could not be matched " +
                    s"to a location related to the index of the data.")
            _start = this._endog_len + _start
        }
        if (_end < 0) {
            require (this._endog_len >= _end.abs,
                    s"The `end` argument could not be matched to " +
                    s"a location related to the index of the data.")
            _end = this._endog_len + _end
        }
        require (_end >= _start,
            s"In prediction `end` must not be less than `start`!")
        val fs = FrovedisServer.getServerInstance()
        val arima_pred:Array[Double] = JNISupport.arimaPredict(fs.master_node, 
                                        _start, _end, 
                                        this.modelId, this.dtype)
        val info = JNISupport.checkServerException();
        if (info != "") throw new java.rmi.ServerException(info)
        val spark = SparkSession.builder().getOrCreate()
        import spark.implicits._
        return (((this.start_offset + (_start * this.freq)) to
                 (this.start_offset + (_end * this.freq)) by 
                 this.freq) 
                zip arima_pred)
               .toDF("index", "predicted_mean")
    }

    def forecast(steps:Long = 1): 
        DataFrame = {
        require (steps > 0,
                s"In prediction `end` must not be less than `start`!")
        val start:Long = this.end_index + 1
        val end:Long = this.end_index + steps
        val fs = FrovedisServer.getServerInstance()
        val fcast:Array[Double] = JNISupport.arimaForecast(fs.master_node, 
                                   steps, this.modelId, this.dtype)
        val info = JNISupport.checkServerException();
        if (info != "") throw new java.rmi.ServerException(info)
        val spark = SparkSession.builder().getOrCreate()
        import spark.implicits._
        return (((this.end_index + this.freq) to
                 (this.end_index + (steps * this.freq)) by 
                 this.freq) 
                zip fcast)
               .toDF("index", "predicted_mean")
    }
}

class ARIMA(var order:List[Int] = List(1, 0, 0),
            var seasonal:Long = 0, var autoArima:Boolean = false, 
            var solver:String = "lapack", var targetCol:String = null, 
            var indexCol:String = null, verbose:Int = 0) 
                extends java.io.Serializable {
    def validate_params():Unit = {
        require(this.order.length == 3,
            s"`order` argument must be an iterable with " +
            s"three elements, but got ${this.order.length} elements.")
        require(this.order(0) >= 1,
            s"`AR order` must not be less than 1, " +
            s"got ${this.order(0)}.")
        require(this.order(1) >= 0,
            s"`Diff order` must not be less than 0, " +
            s"got ${this.order(1)}.")
        require(this.order(2) >= 0,
            s"`MA order` must not be less than 0, " +
            s"got ${this.order(2)}.")
        require(this.seasonal >= 0,
            s"'seasonal' differencing interval cannot be negative, " +
            s"got ${this.seasonal}.")
        if (this.solver == "sag") this.solver = "sgd" 
        else {
            require(List("sgd", "lapack", "lbfgs", "scalapack")
                .contains(this.solver),
            s"'solver' argument must be sag, lapack, lbfgs or scalapack "+
            s"but, got ${this.solver}.")
        }
    }

    def setOrder(order:List[Int]):this.type = {
        require(order.length == 3,
            s"`order` argument must be an iterable with " +
            s"three elements, but got ${order.length} elements.")
        this.order = order
        return this
    }

    def setSolver(solver:String):this.type = {
        if (solver == "sag") this.solver = "sgd"
        else {
            require(List("sgd", "lapack", "lbfgs", "scalapack")
                .contains(solver),
            s"'solver' argument must be sag, lapack, lbfgs or scalapack "+
            s"but, got ${solver}.")
            this.solver = solver
        }
        return this
    }

    def setSeasonal(seasonal:Long):this.type = {
        require((seasonal >= 0),
            s"'seasonal' differencing interval cannot be negative, " +
            s"got ${seasonal}.")
        this.seasonal = seasonal
        return this
    }

    def setAutoArima(autoArima:Boolean):this.type = {
        this.autoArima = autoArima
        return this
    }

    def setIndexCol(indexCol:String):this.type = {
        this.indexCol = indexCol
        return this
    }

    def setTargetCol(targetCol:String):this.type = {
        this.targetCol = targetCol
        return this
    }

    def _get_frequency(df:DataFrame, 
                      indexCol:String): Int = {
      val spark = SparkSession.builder().getOrCreate()
      import spark.implicits._
      var target = df
      val tmp = target.select(col(indexCol), 
                              (lead(indexCol, 1)
                               .over(Window.orderBy(lit(1))))
                                    .as("next"))
                      .withColumn("diff",  $"next" - col(indexCol))
                      .cache
      val zero_cnt = tmp.filter(col("diff") === 0).count()
      if (zero_cnt != 0) return 0
      val cnt = tmp.select(countDistinct("diff")).first.getLong(0)
      if (cnt != 1) return 0
      return tmp.select($"diff".cast("int")).first.getInt(0)
    }

    def validate_cols(cols_n:Int, cols:Array[String]):Unit = {
        if (this.indexCol == null && this.targetCol == null) {
            if (cols_n > 2) {
                throw new IllegalStateException(
                    s"Unable to determine target column(and index column)." +
                    s"Provide target column(and index column) if number " +
                    s"of columns is more than two.")
            }else if (cols_n == 2) {
                this.indexCol = cols(0)
                this.targetCol = cols(1)
            }else if (cols_n == 1) {
                this.targetCol = cols(0)
            }else {
                throw new IllegalStateException(
                    s"Unable to determine number of columns in dataframe." +
                    s"Please ensure that dataframe is not empty.")
            }
        }else if (this.targetCol == null && this.indexCol != null) {
            if (cols_n > 2) {
                throw new IllegalStateException(
                    s"Unable to determine target column." +
                    s"Provide target column(and index column) if number " +
                    s"of columns is more than two.")
            }else if (cols_n == 2) {
                if (this.indexCol == cols(0)) this.targetCol = cols(1)
                else if (this.indexCol == cols(1)) this.targetCol = cols(0)
                else {
                    throw new IllegalStateException(
                        s"Unable to locate index column in provided dataframe."+
                        s"Please confirm index column name and try again.")
                }
            }else {
                throw new IllegalStateException(
                    s"Expected two columns in provided dataframe." +
                    s"Please ensure target column is present in dataframe.")
            }
        }
    }

    def fit(endog:org.apache.spark.sql.DataFrame):ARIMAModel = {
        validate_params()
        var freq = 1
        var end_index:Long = 0
        val cols_n = endog.columns.length
        val cols = endog.columns
        var start_offset:Long = 0
        validate_cols(cols_n, cols)
        if (indexCol != null) {
            start_offset = endog.select(col(indexCol).cast("long"))
                                   .head(1)(0).getLong(0)
            freq = this._get_frequency(endog, indexCol)
            end_index = endog.select(col(indexCol).cast("long"))
                             .tail(1)(0).getLong(0)
        }
        if (freq == 0) {
            println("Unsupported index received, reverting to default 0 based"+
                    " index with frequency 1")
            freq = 1
            start_offset = 0
            end_index = endog.count() - 1
        }
        val sc = SparkContext.getOrCreate()
        var trgt_dbl_col = column(targetCol).cast("double")
        val endog_rdd = endog.select(trgt_dbl_col)
                             .rdd.map(x => x.getDouble(0))
        val endog_len = endog_rdd.count()
        if (endog_len 
         < (this.order(0) + this.order(1) + this.order(2) + this.seasonal + 2))
            throw new IllegalStateException(
                s"Number of samples in input is too " +
                s"less for time series analysis!")
        val endog_dv_p =DoubleDvector.get(endog_rdd)
        return _fit_impl(endog_dv_p, endog_len, DTYPE.DOUBLE, freq, 
                         end_index, start_offset)
    }

    def fit(endog:RDD[Float])(implicit tag: TypeTag[Float]):ARIMAModel = {
        validate_params()
        val endog_len = endog.count()
        if (endog_len 
         < (this.order(0) + this.order(1) + this.order(2) + this.seasonal + 2))
            throw new IllegalStateException(
                s"Number of samples in input is too " +
                s"less for time series analysis!")
        val endog_dv_p = FloatDvector.get(endog)
        return _fit_impl(endog_dv_p, endog_len, DTYPE.FLOAT)
    }

    def fit(endog:RDD[Double]):ARIMAModel = {
        validate_params()
        val endog_len = endog.count()
        if (endog_len 
         < (this.order(0) + this.order(1) + this.order(2) + this.seasonal + 2))
            throw new IllegalStateException(
                s"Number of samples in input is too " +
                s"less for time series analysis!")
        val endog_dv_p = DoubleDvector.get(endog)
        return _fit_impl(endog_dv_p, endog_len, DTYPE.DOUBLE)
    }

    def _fit_impl(endog:Long, endog_len:Long, dtype:Short, freq:Int = 1, 
             end_index:Long = 0, start_offset:Long = 0):ARIMAModel = {
        var _end_index = end_index
        val modelId = ModelID.get()
        val fs = FrovedisServer.getServerInstance()
        JNISupport.arimaFit(fs.master_node, endog,
                             this.order(0), this.order(1), this.order(2),
                             this.seasonal, this.autoArima, this.solver,
                             this.verbose, modelId, dtype)
        val info = JNISupport.checkServerException()
        if (info != "") throw new java.rmi.ServerException(info)
        if (_end_index == 0) _end_index = endog_len - 1
        return new ARIMAModel(modelId, dtype, endog_len, freq, _end_index, 
                              start_offset)
    }
}
