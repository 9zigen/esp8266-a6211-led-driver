if (process.env.NODE_ENV === 'production') {
  module.exports = {
    pluginOptions: {
      'style-resources-loader': {
        preProcessor: 'scss',
        patterns: []
      }
    },

    chainWebpack: (config) => {
      config.optimization.delete('splitChunks')
    },

    // baseUrl: undefined,
    assetsDir: undefined,
    runtimeCompiler: undefined,
    productionSourceMap: false,
    parallel: undefined,
    css: { extract: false },
    filenameHashing: false,

    // outputDir: '../data'
    outputDir: undefined,

    lintOnSave: undefined
  }
}
