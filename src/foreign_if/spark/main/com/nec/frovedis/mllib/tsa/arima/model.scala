package com.nec.frovedis.mllib.tsa.arima;

import com.nec.frovedis.Jexrpc.{FrovedisServer,JNISupport}
import com.nec.frovedis.matrix.{DTYPE, FloatDvector, DoubleDvector}
import com.nec.frovedis.mllib.{M_KIND,ModelID,GenericModel}
import org.apache.spark.SparkContext
import org.apache.spark.rdd.RDD
import scala.reflect.runtime.universe._

class ARIMAModel(modelId:Int, dtype:Short, _endog_len:Long)
    extends GenericModel(modelId, M_KIND.ARM) {
    override def release():Unit = {
        val fs = FrovedisServer.getServerInstance()
        JNISupport.releaseARIMAModel(fs.master_node, 
                                     this.modelId, this.dtype)
        val info = JNISupport.checkServerException()
        if (info != "") throw new java.rmi.ServerException(info)
    }
    def fittedvalues():Array[Double] = {
        var ret:Array[Double] = null
        val fs = FrovedisServer.getServerInstance()
        ret = JNISupport.getFittedVector(fs.master_node, 
                                           this.modelId, this.dtype)
        val info = JNISupport.checkServerException()
        if (info != "") throw new java.rmi.ServerException(info)
        return ret
    }
    def predict(start:Long = 0, end:Long = this._endog_len - 1, 
                dynamic:Boolean = false): 
        Array[Double] = {
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
        require (dynamic == false,
            s"Currently, predict() does not support dynamic = true!")
        val fs = FrovedisServer.getServerInstance()
        val arima_pred:Array[Double] = JNISupport.arimaPredict(fs.master_node, 
                                        _start, _end, this.modelId, this.dtype)
        val info = JNISupport.checkServerException();
        if (info != "") throw new java.rmi.ServerException(info)
        return arima_pred
    }
    def forecast(steps:Long = 1): 
        Array[Double] = {
        require (steps > 0,
                s"In prediction `end` must not be less than `start`!")
        val fs = FrovedisServer.getServerInstance()
        val fcast:Array[Double] = JNISupport.arimaForecast(fs.master_node, 
                                   steps, this.modelId, this.dtype)
        val info = JNISupport.checkServerException();
        if (info != "") throw new java.rmi.ServerException(info)
        return fcast
    }
}

class ARIMA(var order:List[Int] = List(1, 0, 0),
            dates:Any = None, freq:String = "",
            validate_specification:Boolean = true, var seasonal:Long = 0,
            var auto_arima:Boolean = false, var solver:String = "lapack",
            verbose:Int = 0) extends java.io.Serializable {
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

    def setAutoArima(auto_arima:Boolean):this.type = {
        this.auto_arima = auto_arima
        return this
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
        return _fit(endog_dv_p, endog_len,  DTYPE.FLOAT)
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
        return _fit(endog_dv_p, endog_len, DTYPE.DOUBLE)
    }

    def _fit(endog:Long, endog_len:Long, dtype:Short):ARIMAModel = {
        val modelId = ModelID.get()
        val fs = FrovedisServer.getServerInstance()
        JNISupport.arimaFit(fs.master_node, endog,
                             this.order(0), this.order(1), this.order(2),
                             this.seasonal, this.auto_arima, this.solver,
                             this.verbose, modelId, dtype)
        val info = JNISupport.checkServerException();
        if (info != "") throw new java.rmi.ServerException(info);
        return new ARIMAModel(modelId, dtype, endog_len)
    }
}
