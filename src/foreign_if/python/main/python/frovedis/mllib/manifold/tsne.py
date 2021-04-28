""" This is the exact t-SNE implementation """

from ...base import BaseEstimator
from ...exrpc.server import FrovedisServer
from ...exrpc.rpclib import compute_tsne, check_server_exception
from ...matrix.ml_data import FrovedisFeatureData
from ...matrix.dense import FrovedisRowmajorMatrix
from ...matrix.dtype import TypeUtil

class TSNE(BaseEstimator):
    """t-distributed Stochastic Neighbor Embedding (TSNE)
    t-SNE [1] is a tool to visualize high-dimensional data. It converts
    similarities between data points to joint probabilities and tries
    to minimize the Kullback-Leibler divergence between the joint
    probabilities of the low-dimensional embedding and the
    high-dimensional data. t-SNE has a cost function that is not convex,
    i.e. with different initializations we can get different results.
    
    Parameters
    ----------
    n_components : int, optional (default: 2)
        Dimension of the embedded space.

    perplexity : float, optional (default: 30)
        The perplexity is related to the number of nearest neighbors that
        is used in other manifold learning algorithms. Larger datasets
        usually require a larger perplexity. Consider selecting a value
        between 5 and 50. Different values can result in significanlty
        different results.

    early_exaggeration : float, optional (default: 12.0)
        Controls how tight natural clusters in the original space are in
        the embedded space and how much space will be between them. For
        larger values, the space between natural clusters will be larger
        in the embedded space. Again, the choice of this parameter is not
        very critical. If the cost function increases during initial
        optimization, the early exaggeration factor or the learning rate
        might be too high.

    learning_rate : float, optional (default: 200.0)
        The learning rate for t-SNE is usually in the range [10.0, 1000.0]. If
        the learning rate is too high, the data may look like a 'ball' with any
        point approximately equidistant from its nearest neighbours. If the
        learning rate is too low, most points may look compressed in a dense
        cloud with few outliers. If the cost function gets stuck in a bad local
        minimum increasing the learning rate may help.

    n_iter : int, optional (default: 1000)
        Maximum number of iterations for the optimization. Should be at
        least 250.

    n_iter_without_progress : int, optional (default: 300)
        Maximum number of iterations without progress before we abort the
        optimization, used after 250 initial iterations with early
        exaggeration. Note that progress is only checked every 50 iterations so
        this value is rounded to the next multiple of 50.

    min_grad_norm : float, optional (default: 1e-7)
        If the gradient norm is below this threshold, the optimization will
        be stopped.

    metric : string, optional (default : "euclidean")
        The metric to use when calculating distance between instances in a
        feature array. Possible options are "precomputed" and "euclidean".
        If metric is "precomputed", X is assumed to be a distance matrix.
        The default is "euclidean" which is interpreted as squared euclidean
        distance.

    init : string, optional (default: "random")
        Initialization of embedding. Currentl only "random" is supported.

    verbose : int, optional (default: 0)
        Verbosity level.

    random_state : Parameter not supported.

    method : string (default: "exact")
        T-SNE implementation method to use. Currently only "exact" method is
        supported.

    angle : Parameter not supported.

    n_jobs : Parameter not supported.

    Attributes
    ----------
    embedding_ : array-like, shape (n_samples, n_components)
        Stores the embedding vectors.

    kl_divergence_ : double
        Kullback-Leibler divergence after optimization.

    n_iter_ : int
        Number of iterations run.
    """

    def __init__(self, n_components=2, perplexity=30.0,
                 early_exaggeration=12.0, learning_rate=200.0, n_iter=1000,
                 n_iter_without_progress=300, min_grad_norm=1e-7,
                 metric="euclidean", init="random", verbose=0,
                 random_state=None, method="exact", angle=0.5,
                 n_jobs=None):
        """Initialization sequence for TSNE"""
        self.n_components = n_components
        self.perplexity = perplexity
        self.early_exaggeration = early_exaggeration
        self.learning_rate = learning_rate
        self.n_iter = n_iter
        self.n_iter_without_progress = n_iter_without_progress
        self.min_grad_norm = min_grad_norm
        self.metric = metric
        self.init = init
        self.verbose = verbose
        self.random_state = random_state
        self.method = method
        self.angle = angle
        self.n_jobs = n_jobs
        # Some Attribute Parameters
        self._embedding = None
        self._kl_divergence = None
        self._n_iter = None

    def _validate_parameters(self):
        """Private function to validate hyper parameters."""
        if self.n_iter < 250:
            raise ValueError("n_iter should be at least 250")
        if self.n_components != 2:
            raise ValueError("Currently Frovedis TSNE supports n_components = 2")
        if self.perplexity < 0:
            raise ValueError("perplexity = {} should be not be negative."\
                             .format(self.perplexity))
        if self.early_exaggeration < 1.0:
            raise ValueError("early_exaggeration must be at least 1, but is {}"
                             .format(self.early_exaggeration))
        if self.learning_rate < 0:
            raise ValueError("learning_rate = {} should be positive"
                             .format(self.learning_rate))

    def _fit(self, X):
        """Private function to fit the model using X as training data."""
        # validate hyper parameters
        self._validate_parameters()
        inp_data = FrovedisFeatureData(X, \
                     caller = "[" + self.__class__.__name__ + "] fit: ",\
                     dense_kind='rowmajor', densify=True)
        input_x = inp_data.get()
        x_dtype = inp_data.get_dtype()
        (host, port) = FrovedisServer.getServerInstance()
        res = compute_tsne(host, port, input_x.get(), self.perplexity,
                           self.early_exaggeration, self.min_grad_norm,
                           self.learning_rate, self.n_components, self.n_iter,
                           self.n_iter_without_progress, 
                           self.metric.encode('ascii'), 
                           self.method.encode('ascii'),
                           self.init.encode('ascii'), 
                           self.verbose, x_dtype)
        excpt = check_server_exception()
        if excpt["status"]:
            raise RuntimeError(excpt["info"])
        embedding_ptr = res['embedding_ptr']
        n_samples = res["n_samples"]
        n_comps = res["n_comps"]
        embedding_mat = {'dptr': embedding_ptr,
                         'nrow': n_samples,
                         'ncol': n_comps}
        embedding = FrovedisRowmajorMatrix(mat=embedding_mat, dtype=TypeUtil. \
                                           to_numpy_dtype(x_dtype))
        self._n_iter = res['n_iter_']
        self._kl_divergence = res['kl_divergence_']
        if inp_data.is_movable():
            self._embedding = embedding.to_numpy_array()
        else:
            self._embedding = embedding
        return self

    def fit(self, X, y=None):
        """Fit X into an embedded space.

        Parameters
        ----------
        X : array, shape (n_samples, n_features) or (n_samples, n_samples)
            If the metric is 'precomputed' X must be a square distance
            matrix. Otherwise it contains a sample per row. If the method
            is 'exact', X may be a sparse matrix of type 'csr', 'csc'
            or 'coo'.

        y : Ignored
        """
        self.fit_transform(X)
        return self

    def fit_transform(self, X, y=None):
        """Fit X into an embedded space and return that transformed
        output.

        Parameters
        ----------
        X : array, shape (n_samples, n_features) or (n_samples, n_samples)
            If the metric is 'precomputed' X must be a square distance
            matrix. Otherwise it contains a sample per row. If the method
            is 'exact', X may be a sparse matrix of type 'csr', 'csc'
            or 'coo'.

        y : Ignored

        Returns
        -------
        X_new : array, shape (n_samples, n_components)
            Embedding of the training data in low-dimensional space.
        """
        self._fit(X)
        return self._embedding

    @property
    def embedding_(self):
        """embedding_ getter"""
        if self._embedding is None:
            raise ValueError("embedding_ is called before fit")
        return self._embedding

    @embedding_.setter
    def embedding_(self, val):
        """Setter method for embeddings """
        raise AttributeError(\
        "attribute 'embedding_' of TSNE is not writable")

    @property
    def kl_divergence_(self):
        """kl_divergence_ getter"""
        if self._kl_divergence is None:
            raise ValueError("kl_divergence_ is called before fit")
        return self._kl_divergence

    @kl_divergence_.setter
    def kl_divergence_(self, val):
        """Setter method for kl_divergence """
        raise AttributeError(\
        "attribute 'kl_divergence_' of TSNE is not writable")

    @property
    def n_iter_(self):
        """n_iter_ getter"""
        if self._n_iter is None:
            raise ValueError("n_iter_ is called before fit")
        return self._n_iter

    @n_iter_.setter
    def n_iter_(self, val):
        """Setter method for n_iter """
        raise AttributeError(\
        "attribute 'n_iter_' of TSNE is not writable")
